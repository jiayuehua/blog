---
layout: post
title: "`compressed_vector_traits`"
date: 2019-02-16 00:01:00 +0000
tags:
  library-design
  metaprogramming
---

From Slack earlier tonight:

    #include <bit>  // P0237R9, targeting C++2a

    namespace nonstd {

    template<class T>
    struct compressed_vector_traits;

    template<>
    struct compressed_vector_traits<bool> {
        static constexpr int width_in_bits = 1;
        static bool from_bits(std::bit_pointer<const std::byte> first) {
            return *first;
        }
        static void to_bits(bool value, std::bit_pointer<std::byte> first) {
            *first = value;
        }
    };

    template<class T, class Alloc = std::allocator<std::byte>,
             class Traits = compressed_vector_traits<T>>
    class compressed_vector {
        // ...
    };

    } // namespace nonstd

And then:

    struct Cardinal {
        enum Which : int { NORTH, SOUTH, EAST, WEST };
        Which direction : 2;
    };

    template<>
    struct nonstd::compressed_vector_traits<Cardinal> {
        static constexpr int width_in_bits = 2;
        static Cardinal from_bits(std::bit_pointer<const std::byte> first) {
            int result = *first++;
            result = (2 * result) + *first++;
            return Cardinal{ Which(result) };
        }
        static void to_bits(const Cardinal& value, std::bit_pointer<std::byte> first) {
            *first++ = (int(value.direction) / 2);
            *first++ = (int(value.direction) % 2);
        }
    };

    // uses only 1MB, not 8MB
    nonstd::compressed_vector<bool> bools(8'000'000, true);

    // uses only 2MB, not 8MB
    nonstd::compressed_vector<Cardinal> cards(8'000'000, Cardinal{NORTH});

Note to enthusiastic readers: This is _not_ a fully worked idea. It has never been implemented
as far as I know. Please do not propose it for standardization by WG21!

But if you find it useful, please do go implement it and publish your implementation
on GitHub or something.
