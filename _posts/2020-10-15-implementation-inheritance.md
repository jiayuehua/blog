---
layout: post
title: 'A case study in implementation inheritance'
date: 2020-10-15 00:01:00 +0000
tags:
  classical-polymorphism
  war-stories
excerpt: |
  The more I deal with classically polymorphic code, the more I appreciate the
  "modern" idioms that have grown up around it — the Non-Virtual Interface Idiom;
  [Liskov substitutability](https://en.wikipedia.org/wiki/Liskov_substitution_principle);
  Scott Meyers' dictum that classes should be either abstract or final; the
  rule that every hierarchy should have exactly two levels; and the rule that
  [base classes express commonality of _interface_](/blog/2020/10/09/when-to-derive-and-overload/),
  not reuse of _implementation_.

  Today I'd like to present a chunk of code that shows how "implementation inheritance"
  ended up causing trouble, and the particular patterns I applied to disentangle it.
  Unfortunately, the offending code is pretty messy, so I'll use a simplified and
  domain-shifted example; and I'll try to build it up in stages.
---

The more I deal with classically polymorphic code, the more I appreciate the
"modern" idioms that have grown up around it — the Non-Virtual Interface Idiom;
[Liskov substitutability](https://en.wikipedia.org/wiki/Liskov_substitution_principle);
Scott Meyers' dictum that classes should be either abstract or final; the
rule that every hierarchy should have exactly two levels; and the rule that
[base classes express commonality of _interface_](/blog/2020/10/09/when-to-derive-and-overload/),
not reuse of _implementation_.

Today I'd like to present a chunk of code that shows how "implementation inheritance"
ended up causing trouble, and the particular patterns I applied to disentangle it.
Unfortunately, the offending code is pretty messy, so I'll use a simplified and
domain-shifted example; and I'll try to build it up in stages.


## Step 1: Transactions

Let our domain be "banking transactions." We have a classically polymorphic inheritance hierarchy,
because of course we do.

    class Txn { ... };
    class DepositTxn : public Txn { ... };
    class WithdrawalTxn : public Txn { ... };
    class TransferTxn : public Txn { ... };

All kinds of transaction have certain APIs in common, and then each type also has its own
idiosyncratic APIs.

    class Txn {
    public:
        AccountNumber account() const;
        std::string name_on_account() const;
        Money amount() const;
    private:
        // virtual stuff
    };

    class DepositTxn : public Txn {
    public:
        std::string name_of_customer() const;
    };

    class TransferTxn : public Txn {
    public:
        AccountNumber source_account() const;
    };


## Step 2: Transaction filters

What our software is actually doing, though, isn't _executing_ transactions; it's _monitoring_ them
so that it can flag suspicious transactions. The software's human operator can set up filters to
match against certain criteria, like "flag all transactions larger than $10,000" or "flag all
transactions where the person's name is on watchlist W." Internally, we represent the various
operator-configured filter types as a classically polymorphic hierarchy, because of course we do.

    class Filter { ... };
    class AmountGtFilter : public Filter { ... };
    class NameWatchlistFilter : public Filter { ... };
    class AccountWatchlistFilter : public Filter { ... };
    class DifferentCustomerFilter : public Filter { ... };
    class AndFilter : public Filter { ... };
    class OrFilter : public Filter { ... };

All filters have exactly the same public API.

    class Filter {
    public:
        bool matches(const Txn& txn) const {
            return do_matches(txn);
        }
    private:
        virtual bool do_matches(const Txn&) const = 0;
    };

Here's an example of a simple filter:

    class AmountGtFilter : public Filter {
    public:
        explicit AmountGtFilter(Money x) : amount_(x) { }
    private:
        bool do_matches(const Txn& txn) const override {
            return txn.amount() > amount_;
        }

        Money amount_;
    };


## Step 3: The first misstep

It turns out that some filters really want to access those idiosyncratic transaction-specific
APIs I mentioned earlier. Let's say that `DifferentCustomerFilter` wants to flag
any transaction where the name of the customer who made the transaction was different from the
name on the account. For the sake of this example, our bank strictly enforces that
only the account holder is ever allowed to withdraw money from their account. So
only `class DepositTxn` even bothers to record the name of the customer who made the
transaction.

    class DifferentCustomerFilter : public Filter {
        bool do_matches(const Txn& txn) const override {
            if (auto *dtxn = dynamic_cast<const DepositTxn*>(&txn)) {
                return dtxn->name_of_customer() != dtxn->name_on_account();
            } else {
                return false;
            }
        }
    };

This is a classic abuse of `dynamic_cast`! (It's classic because it's so darn common in real code.)
We could use the facility from ["Classically polymorphic `visit`"](/blog/2020/09/29/oop-visit/) (2020-09-29)
to tighten up the code, but in this case it's no improvement:

    class DifferentCustomerFilter : public Filter {
        bool do_matches(const Txn& txn) const override {
            my::visit<DepositTxn>(txn, [](const auto& dtxn) {
                return dtxn.name_of_customer() != dtxn.name_on_account();
            }, [](const auto&) {
                return false;
            });
        }
    };

So the original author of this code got a (sarcasm alert!) bright idea. Let's permit
certain kinds of filters to be written "casewise" instead of "generically." We rewrite
the base `class Filter` like this:

    class Filter {
    public:
        bool matches(const Txn& txn) const {
            return my::visit<DepositTxn, WithdrawalTxn, TransferTxn>(txn, [](const auto& txn) {
                return do_generic(txn) && do_casewise(txn);
            });
        }
    private:
        virtual bool do_generic(const Txn&) const { return true; }
        virtual bool do_casewise(const DepositTxn&) const { return true; }
        virtual bool do_casewise(const WithdrawalTxn&) const { return true; }
        virtual bool do_casewise(const TransferTxn&) const { return true; }
    };

    class LargeAmountFilter : public Filter {
        bool do_generic(const Txn& txn) const override {
            return txn.amount() > Money::from_dollars(10'000);
        }
    };

    class DifferentCustomerFilter : public Filter {
        bool do_casewise(const DepositTxn& dtxn) const override {
            return dtxn.name_of_customer() != dtxn.name_on_account();
        }
        bool do_casewise(const WithdrawalTxn&) const override { return false; }
        bool do_casewise(const TransferTxn&) const override { return false; }
    };

This "clever" tactic reduces the amount of code we have to write in `DifferentCustomerFilter`.
But it violates one of our OOP principles: the rule against _implementation inheritance_.
The function `Filter::do_generic(const Txn&)` is neither pure nor final. This is going
to get us into trouble.

> By the way, I claim that `my::visit` does win over a series of `dynamic_cast`s in this particular
> situation. Also notice the generic lambda dispatching among three functions all named `do_casewise`;
> the use of an overload set here follows the guideline laid out in
> ["Inheritance is for sharing an interface (and so is overloading)"](/blog/2020/10/09/when-to-derive-and-overload/) (2020-10-09).


## Step 4: The second misstep

Now we'll make a filter that checks whether the account holder is on any government watchlist.
There are several lists we want to check.

    class NameWatchlistFilter : public Filter {
    protected:
        bool is_flagged(std::string_view name) const {
            for (const auto& list : watchlists_) {
                if (std::find(list.begin(), list.end(), name) != list.end()) {
                    return true;
                }
            }
            return false;
        }

    private:
        bool do_generic(const Txn& txn) const override {
            return is_flagged(txn.name_on_account());
        }

        std::vector<std::list<std::string>> watchlists_;
    };

Oh, and we should also make a filter that casts a wider net — checking both the account holder and the
customer's own name. Here again the original author gets a (sarcasm alert!) clever idea. Instead of
duplicating the `is_flagged` logic, let's inherit it. Inheritance is (sarcasm alert!) for code reuse, right?

    class WideNetFilter : public NameWatchlistFilter {
        bool do_generic(const Txn& txn) const override {
            return true;
        }
        bool do_casewise(const DepositTxn& txn) const override {
            return is_flagged(txn.name_on_account()) || is_flagged(txn.name_of_customer());
        }
        bool do_casewise(const WithdrawalTxn& txn) const override {
            return is_flagged(txn.name_on_account());
        }
        bool do_casewise(const TransferTxn& txn) const override {
            return is_flagged(txn.name_on_account());
        }
    };

Notice the awfully confusing architecture here. `NameWatchlistFilter` overrides `do_generic` to check only the
account holder's name; then `WideNetFilter` overrides it back to the original definition.
(If `WideNetFilter` did not override it back, then `WideNetFilter` would misbehave for any deposit transaction
where `name_on_account()` is unflagged and `name_of_customer()` is flagged.) It's confusing, but it works...
for now.


## Step 5: A series of unfortunate events

The particular way this technical debt came back to bite us was that we needed to add a new transaction type.
Let's make a class to represent fees and interest payments initiated by the bank itself.

    class FeeTxn : public Txn { ... };

    class Filter {
    public:
        bool matches(const Txn& txn) const {
            return my::visit<DepositTxn, WithdrawalTxn, TransferTxn, FeeTxn>(txn, [](const auto& txn) {
                return do_generic(txn) && do_casewise(txn);
            });
        }
    private:
        virtual bool do_generic(const Txn&) const { return true; }
        virtual bool do_casewise(const DepositTxn&) const { return true; }
        virtual bool do_casewise(const WithdrawalTxn&) const { return true; }
        virtual bool do_casewise(const TransferTxn&) const { return true; }
        virtual bool do_casewise(const FeeTxn&) const { return true; }
    };

The crucial step: We forget to update `WideNetFilter` by adding an overrider for
`WideNetFilter::do_casewise(const FeeTxn&) const`. This probably seems like an unforgivable oversight,
in this toy example; but in the real code — where a few dozen lines separate each of the existing
overriders, and the Non-Virtual Interface Idiom is not followed so religiously — I think it's all too easy to
see `class WideNetFilter : public NameWatchlistFilter`, overriding both `do_generic` _and_ `do_casewise`,
and think "oh, this is confusing, I'll come back to it" (and then never come back to it).

Anyway, I hope you agree by this point that we've created a monster. How do we de-monsterify it?


## Refactoring away implementation inheritance, step 1

To eliminate the implementation inheritance in `Filter::do_casewise`, we'll apply Scott Meyers' dictum
that every virtual method should be either pure or (logically) final. In this case it's a tradeoff,
because we're simultaneously breaking the rule that hierarchies should be shallow. We introduce an
intermediate class:

    class Filter {
    public:
        bool matches(const Txn& txn) const {
            return do_generic(txn);
        }
    private:
        virtual bool do_generic(const Txn&) const = 0;
    };

    class CasewiseFilter : public Filter {
        bool do_generic(const Txn&) const final {
            return my::visit<DepositTxn, WithdrawalTxn, TransferTxn>(txn, [](const auto& txn) {
                return do_casewise(txn);
            });
        }

        virtual bool do_casewise(const DepositTxn&) const = 0;
        virtual bool do_casewise(const WithdrawalTxn&) const = 0;
        virtual bool do_casewise(const TransferTxn&) const = 0;
    };

Now, filters that want to provide casewise handling for each possible transaction type can simply inherit
from `CasewiseFilter`. Filters whose implementations are generic continue to inherit directly from `Filter`.

    class LargeAmountFilter : public Filter {
        bool do_generic(const Txn& txn) const override {
            return txn.amount() > Money::from_dollars(10'000);
        }
    };

    class DifferentCustomerFilter : public CasewiseFilter {
        bool do_casewise(const DepositTxn& dtxn) const override {
            return dtxn.name_of_customer() != dtxn.name_on_account();
        }
        bool do_casewise(const WithdrawalTxn&) const override { return false; }
        bool do_casewise(const TransferTxn&) const override { return false; }
    };

If someone adds a new transaction type, and modifies `CasewiseFilter` to include a new overload of
`do_casewise`, we'll see that `DifferentCustomerFilter` has become an abstract class: we _must_ deal
with the new transaction type. The compiler now helps to enforce the rules of our architecture,
where before it silently concealed our mistakes.

Notice also that the definition of `WideNetFilter` in terms of `NameWatchlistFilter` is now impossible.


## Refactoring away implementation inheritance, step 2

To eliminate the implementation inheritance in `WideNetFilter`, we apply the Single Responsibility
Principle. Right now `NameWatchlistFilter` has two jobs: to be a filter in its own right, and to
own the `is_flagged` facility. Let's split out `is_flagged` into a separate
`class WatchlistGroup`, which doesn't have to worry about conforming to the API of
`class Filter` because it's not a filter — it's just a helpful utility class.

    class WatchlistGroup {
    public:
        bool is_flagged(std::string_view name) const {
            for (const auto& list : watchlists_) {
                if (std::find(list.begin(), list.end(), name) != list.end()) {
                    return true;
                }
            }
            return false;
        }
    private:
        std::vector<std::list<std::string>> watchlists_;
    };

Now, `WideNetFilter` can use `is_flagged` without inheriting from `NameWatchlistFilter`.
In both filters we can follow the guideline to _prefer composition over inheritance_ (because
composition is a tool for code reuse; inheritance is not).

    class NameWatchlistFilter : public Filter {
        bool do_generic(const Txn& txn) const override {
            return wg_.is_flagged(txn.name_on_account());
        }

        WatchlistGroup wg_;
    };

    class WideNetFilter : public CasewiseFilter {
        bool do_casewise(const DepositTxn& txn) const override {
            return wg_.is_flagged(txn.name_on_account()) || wg_.is_flagged(txn.name_of_customer());
        }
        bool do_casewise(const WithdrawalTxn& txn) const override {
            return wg_.is_flagged(txn.name_on_account());
        }
        bool do_casewise(const TransferTxn& txn) const override {
            return wg_.is_flagged(txn.name_on_account());
        }

        WatchlistGroup wg_;
    };

Again, if someone adds a new transaction type, and modifies `CasewiseFilter` to include a new overload of
`do_casewise`, we'll see that `WideNetFilter` has become an abstract class: we _must_ deal
with the new transaction type in `WideNetFilter` (but not in `NameWatchlistFilter`). It's as if the
compiler is keeping a to-do list for us!


## Conclusions

This example has been anonymized and _extremely_ simplified, compared to the original code I was working on.
However, the general shape of the class hierarchy is accurate, as is the wacky `do_generic(txn) && do_casewise(txn)`
logic from the original code. The main thing that I suspect was lost in translation is just how easy it was
for that `FeeTxn` bug to hide in the old design. Seeing this simplified version laid out (and motivated in
easily digestable steps!) almost makes me wonder — was the original
code really _so_ bad? Well, maybe not. After all, it did work... for a while!

But I hope you agree that the refactored version, using `CasewiseFilter` and especially
`WatchlistGroup`, is better than the old one. If I had to choose one of these two codebases
to work on, I'd choose the new, implementation-inheritance-free version in a heartbeat.
