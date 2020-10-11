---
layout: post
title: 'A very short war story on too much overloading'
date: 2020-10-11 00:01:00 +0000
tags:
  c++-style
  war-stories
---

On my previous post
["Inheritance is for sharing an interface (and so is overloading)"](/blog/2020/10/09/when-to-derive-and-overload/) (2020-10-09),
a couple of Reddit commenters expressed disbelief at my third section, where I claimed that the
`vector::erase` overload set was poorly designed. They had never run into the common bug

    // Oops!
    v.erase(
        std::remove_if(v.begin(), v.end(), isOdd)
    );

For more on erasing from STL containers, see ["How to erase from an STL container"](/blog/2020/07/08/erase-if/) (2020-07-08).

Here's another example of grief caused by overuse-of-overloading, which I just ran into at work last week.

    class mgr_t : public asio_actor {
        ~~~
        virtual std::string getname() const = 0;
        virtual result_t subscribe(actor::ptr tgt, msghandler handler,
            priority::level prio, const std::string type,
            bool enrich = false) = 0;
        virtual result_t subscribe(actor::ptr tgt, msghandler handler,
            priority::level prio, selector f, unsigned &id,
            bool enrich = false) = 0;
        virtual result_t subscribe(actor::ptr tgt, msghandler handler,
            priority::level prio,
            bool enrich = false) = 0;
        virtual void subscribe_connect(actor::ptr tgt, connhandler handler,
            priority::level prio) = 0;
        virtual void subscribe_disconnect(actor::ptr tgt, connhandler handler,
            priority::level prio) = 0;
        ~~~
    };

And then there's a caller in some other repository that does this:

    this->message_ = std::string("subscription failed");
    return g_mgr->subscribe(ctl_actor::instance(), stats,
        priority::level::high, std::string("queue-stats"));

You might look at those lines and think: _Ah, the casts to `std::string` are redundant. This is just
a Java programmer's attempt to keep track of where "string objects" are created. I can remove them._
You'd be right about the first cast, but it turns out you'd be _wrong_ about the second!

    return g_mgr->subscribe(ctl_actor::instance(), stats,
        priority::level::high, std::string("queue-stats"));

is a call to `mgr_t::subscribe(actor::ptr, msghandler, priority::level, const std::string, bool = false)`. But

    return g_mgr->subscribe(ctl_actor::instance(), stats,
        priority::level::high, "queue-stats");

is a call to `mgr_t::subscribe(actor::ptr, msghandler, priority::level, bool = false)`!
Because `"queue-stats"`, after decaying to `const char*`, would _rather_ implicitly convert
to the built-in type `bool` than to the user-defined type `std::string`. ([Godbolt.](https://godbolt.org/z/Txns3q))

The above snippet exemplifies many sins which I've previously discussed on this blog. See:

* ["Default function arguments are the devil"](/blog/2020/04/18/default-function-arguments-are-the-devil/) (2020-04-18)

* ["`const` is a contract"](/blog/2019/01/03/const-is-a-contract/) (2019-01-03) re the missing ampersand

* ["PSA: `shared_ptr<T>()` is not `make_shared<T>()`"](/blog/2019/03/06/shared-ptr-vs-make-shared-pitfall/) (2019-03-06) re `actor::ptr`

* And of course it should be using the Non-Virtual Interface idiom, but I haven't got a blog post
    dedicated to that subject, yet. See
    ["CppCon 2019 talks are up"](/blog/2019/10/18/quotable-cppcon-talks/#jon-kalb-back-to-basics-object-o) (2019-10-18).

Nevertheless, I think the primary sin here — or at least the "[first among equals](https://en.wikipedia.org/wiki/Primus_inter_pares)"
— is that it has three overloads of `subscribe`, doing radically different things with (some of) their arguments!
