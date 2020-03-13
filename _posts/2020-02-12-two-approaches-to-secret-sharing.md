---
layout: post
title: 'Two approaches to secret sharing: math vs. blockchain'
date: 2020-02-12 00:02:00 +0000
tags:
  esolang
  math
  pearls
---

The other day I rediscovered the cryptographic protocol called
[Shamir's Secret Sharing](https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing),
and it reminded me of something I meant to blog about two years ago.

Suppose we run a bank, and suppose that our bank stores all its gold in one big safe.
And suppose that we have three people on our board of directors. We want to ensure
that a single unscrupulous director cannot open the safe and steal all the gold.
Therefore, we cut three distinct keys, and we craft the safe so that it requires
two of these three keys to be inserted simultaneously, in order to open the safe.

Readers of Michael Crichton novels, or fans of Sean Connery, might recognize this
as more or less the setup to [_The Great Train Robbery_](https://amzn.to/2w6MQed),
except for our "two out of three" requirement. I have not been able to discover any
real-world mechanical analogue to the "two-out-of-three" mechanism.
"Two-out-of-two" dual-key mechanisms are state-of-the-art in safety deposit boxes
(although [physical security is a very different game from cryptographic security](https://duckduckgo.com/?q=philip+poniz+safety.deposit.box&ia=web)),
and [trapped-key interlocks](https://en.wikipedia.org/wiki/Trapped_key_interlocking)
also bear some passing similarity.

Anyway, in cryptography, we can solve our "two-out-of-three" keying problem like this.

## Shamir's Secret Sharing

We need a cryptographic analogue for our bank's "gold." Let's say that we're trying
to secure the secret word that would allow us to log in to our online vault.
We want to secure that secret word so that no single unscrupulous director can find it out,
but any pair of directors working together _can_ find it out.
(Of course once the vault is open, both directors in the pair will know the secret word,
and be able to use it any time they want, even unilaterally. So our analogy isn't so great.
But let's run with it.)

First, we hire a trustworthy locksmith. The locksmith chooses a random polynomial
with two terms $$f(x) = a_{1}x + a_0$$, where $$a_0$$ is actually our secret.
This polynomial is defined in some finite field GF(_p_).
The locksmith issues to each director a key in the form of an ordered triple —
$$(1, f(1), p)$$, $$(2, f(2), p)$$, and $$(3, f(3), p)$$.

For example, suppose the secret word is `fish`. Our trustworthy locksmith encodes
that in hex as `0x66697368`. He picks a prime bigger than it; let's say
`0x01'6098b2cd`. He picks a random polynomial in GF(`0x01'6098b2cd`); let's say

    f(x) = 0x1'48270199 * x + 0x66697368

He issues the following keys:
`(1, 0x4df7c234, 0x16098b2cd)`, `(2, 0x35861100, 0x16098b2cd)`, `(3, 0x1d145fcc, 0x16098b2cd)`.

For comparison, if the secret word had been `gold`, our trustworthy locksmith
would have encoded that in hex as `0x676f6c64`, might have picked the random
polynomial `f(x) = 0x1'47a4051b * x + 0x676f6c64`, and might have issued keys
`(1, 0x4e7abeb2, 0x16098b2cd)`, `(2, 0x35861100, 0x16098b2cd)`, `(3, 0x1c91634e, 0x16098b2cd)`.

Then we shoot the locksmith. Now nobody knows the secret word; the only place it's
recorded is as the Y-intercept of the locksmith's polynomial, which nobody knows.

Any director alone cannot reconstruct the polynomial. For example, director 2
cannot reconstruct the polynomial knowing only his personal key `(2, 0x35861100, 0x16098b2cd)`.
However, if two directors get together and share their keys, they can reconstruct
the polynomial. For example, the first two directors can solve for $$a_1$$ like
this:

    a1 = (0x35861100 - 0x4df7c234) % 0x16098b2cd

And knowing $$a_1$$, they can each compute the secret word like this:

    a0 = (0x35861100 - (2 * a1)) % 0x16098b2cd = 0x66697368
    a0 = (0x4df7c234 - (1 * a1)) % 0x16098b2cd = 0x66697368

(As they now have all the information the original locksmith had,
they can now also compute each other's keys, and the key of the third director.)

Let's see another way of tackling the same problem, using a different set of tools.


## The blockchain analogue

I learned about this next bit via Andreas M. Antonopoulos's book
[_Mastering Bitcoin_](https://github.com/bitcoinbook/bitcoinbook/blob/develop/book.asciidoc) (2014),
which is freely available [in translation and/or in source-code form](https://bitcoinbook.info/translations-of-mastering-bitcoin/),
or available for real money [on Amazon](https://amzn.to/38mKJBr).
[Chapter 7](https://github.com/bitcoinbook/bitcoinbook/blob/develop/ch07.asciidoc) discusses
multisignature scripts.

Thanks also to Greg Walker's "[learn me a bitcoin](https://learnmeabitcoin.com/)."

Unlike raw math, Bitcoin is designed to mimic real assets in many ways. So our Bitcoin analogue
of "gold" is not going to be a secret word like `fish`; it's going to be some actual
quantity of bitcoin. We want to secure that bitcoin so that no single unscrupulous director
can transfer it to a different address, but any pair of directors working together
_can_ transfer it to a different address.

The first thing we do is, every director creates their own [ECDSA](https://en.bitcoin.it/wiki/Secp256k1)
keypair, consisting of a public key and a private key. All the directors reveal their public keys.
Let's say that the directors' public keys are these 65-byte vectors:

    04cf291c54e736d412adcf2287bf8bb2cae680b79996e8f6aab8f98648e0419df8
      943ead0e62b5044b533b073b1bc6f2af8ba93ca23e3d9738376824711ef2ad59

    04c38de5add314f1e9d896265e9f436b8e260470f095a04fe66419bff2d24c2ff8
      ecad15a23edbb95db2486f45e41c3bbe331a96c3385f7cc3784d763ced8a1fa7

    04c9ee0cb11ec2cb7ac54a13b07818417822f191d4f789395df0a0ab80eb5d8884
      99a89e6b557ed7aa091fee4889b2bb9c6205512b02362490f129ba105f776c74

Now we create an "unlocking script" in [Bitcoin Script](https://learnmeabitcoin.com/guide/script),
a stack-based language that's part of the Bitcoin protocol. Script is a stack-based language
where its stack doesn't store _numbers_ (like [Befunge](https://esolangs.org/wiki/Befunge)) or
_numbers and/or references_ (like [the JVM](https://docs.oracle.com/javase/specs/jvms/se8/html/jvms-2.html)),
but rather _arbitrary-length byte vectors_. (Actually, the length of a byte vector maxes out
at [520 bytes](https://github.com/bitcoin/bitcoin/blob/99813a97/src/script/script.h#L22-L23) as of this writing.)
Our unlocking script looks like this:

    OP_2
    <push director 1's public key>
    <push director 2's public key>
    <push director 3's public key>
    OP_3
    OP_CHECKMULTISIG

Confusingly, Script doesn't have any particular mnemonic notation for "push a byte vector."
You can push a vector of up to 117 bytes by just placing the vector right into the Script code,
as a Pascal-style string. So hex `02 03 04` means "push the byte vector `0304`," and hex
`01 02` means "push `02`." Since that's so frequently done, Script also provides
a dedicated opcode with a visible mnemonic: hex `52` means "push 2," and is known by the mnemonic `OP_2`.
Other opcodes have nothing to do with pushing; for example, hex `93` means "pop the top two vectors
and [add](https://bitcoin.stackexchange.com/questions/29435/arithmetic-operations-in-bitcoin-op-add-etc) them,"
and hex `AE` means "check signatures for validity in an interesting way to be described later."

So our unlocking script looks like this, in hex, with linebreaks added for readability.
By the way, Simin Chen's [Bitcoin IDE](https://siminchen.github.io/bitcoinIDE/build/editor.html#)
can disassemble scripts like these.

    52
    41 04cf291c54e736d412adcf2287bf8bb2cae680b79996e8f6aab8f98648e0419df8
         943ead0e62b5044b533b073b1bc6f2af8ba93ca23e3d9738376824711ef2ad59
    41 04c38de5add314f1e9d896265e9f436b8e260470f095a04fe66419bff2d24c2ff8
         ecad15a23edbb95db2486f45e41c3bbe331a96c3385f7cc3784d763ced8a1fa7
    41 04c9ee0cb11ec2cb7ac54a13b07818417822f191d4f789395df0a0ab80eb5d8884
         99a89e6b557ed7aa091fee4889b2bb9c6205512b02362490f129ba105f776c74
    53AE

We then create a Bitcoin transaction "locking up" our quantity of bitcoin, by indicating through
the _locking_ script that the output of that transaction is spendable (or "redeemable") only
by someone who presents a copy of our unlocking script, identified by hash, along with the proper
data inputs to make it return `1` when evaluated by a Bitcoin Script interpreter.

Any director alone cannot spend the [UTXO](https://learnmeabitcoin.com/guide/utxo) we thus "locked up."
However, if two directors together want to spend it, they do the following:

Create a new transaction. Agree on the rules for who can spend the output of this
_new_ transaction, and encode them in a locking script which again becomes part of the
new transaction. Hash this new transaction (using double SHA-256, if I understand correctly).
The first director signs a copy of the hash using his private key.
The second director signs a copy of the hash using _his_ private key.
Use these hashes to create a "redeem script" that looks like this:

    OP_0
    <push director 1's cryptographic signature of this transaction>
    <push director 2's cryptographic signature of this transaction>

Send the directors' transaction out to the world. If all goes well, some miner somewhere will
pick it up and incorporate it into a block in the blockchain. Other miners will
not necessarily trust this block; they'll want to verify that it's valid, meaning that
it consists of all valid transactions. To verify that the directors' transaction was valid,
a miner will concatenate its redeem script with its unlocking script:

    OP_0
    <push director 1's cryptographic signature of this transaction>
    <push director 2's cryptographic signature of this transaction>
    OP_2
    <push director 1's public key>
    <push director 2's public key>
    <push director 3's public key>
    OP_3
    OP_CHECKMULTISIG

And then it'll interpret the script — which means push eight byte-vectors onto the stack and then
execute `OP_CHECKMULTISIG`. When the Script interpreter sees this opcode, it pops the top `3` from the stack,
and based on that pops three public keys. Then it pops `2` from the stack, and based on that pops two signatures
(and a trailing `0`, for historical reasons). Finally, it validates the signatures: it checks to see if
director 2's signature is `SIGN(hash_of_transaction)` according to director 3's public key. Nope! It goes on
to the next public key. Is director 2's signature valid according to director 2's public key? Yep! It goes
on to the next public key _and_ the next signature. Is director 1's signature valid according to director
1's public key? Also yep! Having validated all of the signatures it popped, it has succeeded; so it pushes `1`
back onto the stack. Finally, having reached the end of the script, the interpreter stops and checks the top
value on the stack. It's `1` (truthy), so the transaction is considered valid (by every miner who bothers to
examine it).

Now you see how one unscrupulous director, acting alone, cannot steal the gold. He could construct a
new transaction with the "redeem script"

    OP_0
    <push director 1's cryptographic signature of this transaction>

or even

    OP_0
    <push director 1's cryptographic signature of this transaction>
    <push director 1's cryptographic signature of this transaction>

but neither of these inputs, when combined with the true "unlocking script," would result in a truthy value
on the interpreter's stack at the end of the script. The unscrupulous director could try presenting a different
unlocking script altogether, for example

    51
    41 04cf291c54e736d412adcf2287bf8bb2cae680b79996e8f6aab8f98648e0419df8
         943ead0e62b5044b533b073b1bc6f2af8ba93ca23e3d9738376824711ef2ad59
    41 04c38de5add314f1e9d896265e9f436b8e260470f095a04fe66419bff2d24c2ff8
         ecad15a23edbb95db2486f45e41c3bbe331a96c3385f7cc3784d763ced8a1fa7
    41 04c9ee0cb11ec2cb7ac54a13b07818417822f191d4f789395df0a0ab80eb5d8884
         99a89e6b557ed7aa091fee4889b2bb9c6205512b02362490f129ba105f776c74
    53AE

or even

    51

but those unlocking scripts don't have the same hash as the true unlocking script chosen by the directors,
whose hash is incorporated into the directors' original transaction that locked up the funds. Therefore again
no miner will consider the unscrupulous director's new transaction to be valid.


## Conclusion

The mind-bending thing about the Bitcoin approach is that it completely sidesteps all the math
that makes Shamir's scheme work. We don't have to look for some mathematical primitive that can
split up a secret into parts. We just build the "2-of-3 keys required" protocol directly into
the Bitcoin Script interpreter, as a primitive operation!

And then we leave our money out in plain sight, guarded only by a sign that says "Everyone please
take note: Nobody shall be considered to have taken any of this money unless they were accompanied
by at least two of the bank's directors."

How can this be secure? Couldn't anyone "get at" the funds, if they're sitting right there, guarded
only by convention? Well, yes — but what's the use of "getting at" money
if nobody in the world agrees that you have gotten it? In fact (and this is Bitcoin's core insight),
if nobody _agrees_ that you have the money, then in a very real sense you haven't got it at all.
