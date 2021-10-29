---
layout: post
title: "C++ Pub Quiz at CppCon 2020, Game One"
date: 2020-09-16 00:01:00 +0000
tags:
  conferences
  cppcon
excerpt: |
  ![Spoiler for question #4](/blog/images/2020-09-16-anagram-time-1.jpg){: .float-right}
  [CppCon 2020](https://cppcon.org/program2020/) is half over;
  this afternoon I gave [my B2B talk on algebraic data types](https://cppcon2020.sched.com/event/e7Ad/).
  On Friday I give [my B2B talk on concurrency](https://cppcon2020.sched.com/event/e7Ab/),
  and next week I'll deliver
  [my three-day "Classic STL" training course](https://cppcon.org/class-2020-classic-stl/)
  — for which there is still time to register! :)

  Tonight I emceed the first game of the second annual CppCon "C++ Pub Quiz."
  (There's [a second game happening](https://cppcon2020.sched.com/event/eU1G/c-pub-quiz) tomorrow night.)
  What is C++ Pub Quiz, you ask? It's a straightforward trivia night — five rounds,
  ten questions per round, write down your team's answers on a sheet of paper and turn it in
  at the end of the round. High score after five rounds wins the game.

  Seven teams participated in Pub Quiz. Thanks to `std::my_teammates_dont_respond_exception`,
  Team C&#8288;-&#8288;-, Team Don't Care, The French Connection, Team Sea Slug, `DROP TABLE * FROM ANSWERS`,
  Team 0x1010, and Team IFNDR!
---

[CppCon 2020](https://cppcon.org/program2021/) is half over;
this afternoon I gave [my B2B talk on algebraic data types](https://cppcon2020.sched.com/event/e7Ad/).
On Friday I give [my B2B talk on concurrency](https://cppcon2020.sched.com/event/e7Ab/),
and next week I'll deliver
[my three-day "Classic STL" training course](https://cppcon.org/class-2020-classic-stl/)
— for which there is still time to register! :)

Tonight I emceed the first game of the second annual CppCon "C++ Pub Quiz."
(There's [a second game happening](https://cppcon2020.sched.com/event/eU1G/c-pub-quiz) tomorrow night.)
What is C++ Pub Quiz, you ask? It's a straightforward trivia night — five rounds,
ten questions per round, write down your team's answers on a sheet of paper and turn it in
at the end of the round. High score after five rounds wins the game.

Seven teams participated in Pub Quiz. Thanks to `std::my_teammates_dont_respond_exception`,
Team C&#8288;-&#8288;-, Team Don't Care, The French Connection, Team Sea Slug, `DROP TABLE * FROM ANSWERS`,
Team 0x1010, and Team IFNDR!

Our winning team of the night was Team IFNDR, with an impressive score of 35 points.
(Compare to [last year](/blog/2019/09/21/cppcon-2019-pub-quiz/),
when our winning team of the night was Team Undefined Behavior.
This year I told people not to choose "Undefined Behavior" as their team name,
and to be more creative. Serves me right, I guess.)

![Spoiler for question #4](/blog/images/2020-09-16-anagram-time-1.jpg){: .float-right}
Since we're doing it all in cyberspace this year, there were more moving parts than
usual. We met in Remo room `generate_n()`. I sat at a table in the back and used the general chat
to tell people to sit up front, and also to join a Google Meet I'd set up. I read questions
aloud into the Google Meet, and each team was able to discuss their answers via their Remo table chats.
(We did have some teams using text chat, and some amusing snafus when teams accidentally started
typing their answers into the general chat instead of their table-specific chat. But generally
speaking, it went off smoothly.) Each team appointed a team captain to submit their answers via
a Google Form which I set up, which you can see [here](https://forms.gle/oob7sT4ujVmXzGvaA).

During the game, I would mute my mic in Google Meet and hop around from table to table in
Remo to see how the teams were getting along. Everyone seemed to be having a good time!
The only thing was that I could never eavesdrop on The French Connection or whatever team was
at table 1, because their tables were full. Also, I wish I had included a space on the answer
sheet for "table number," so that I could map the team names to the table numbers. I'm
making a fresh answer sheet for Thursday night's game, which will include a blank for "table number."

If you're planning a similar event, take note: Everything takes longer online.
It took us 2.5 hours to get through five rounds of play. But everyone had fun!

----

A sample round of C++ Pub Quiz looks something like this:

1. The standard `<algorithm>` header contains several function templates whose names start
with "`is_`" [SAY: "is underscore"], such as `is_partitioned` and `is_permutation`. Two of these
algorithms have return types other than `bool`. For one point each, name these two algorithms.

2. Name the author of the "Exceptional C++" book series.

3. What is the return type of `std::next_permutation`?

4. What feature of C++ used in the iostreams library anagrams to "I, evil incarnate, hurt"?

5. Name the formerly standard smart pointer type that was introduced in C++98,
deprecated in C++11, and finally removed in C++17.

6. Name the major feature, involving expectations and assurances, which was initially adopted
for the C++20 standard but then pulled back out of the working draft in July 2019.

7. Consider the number of consecutive period characters you can write in a valid C++17 expression,
without using macros or string literals. Which of the following numbers of consecutive period
characters can you not incorporate into such a valid expression: one, two, three, or four?

8. If you wrote down all the overloadable operators in C++20 one after the other, how many times
would you write a greater-than sign — that is, ASCII character 62?

9. The CRTP derives its name from an article titled "Curiously Recurring Template Patterns"
which appeared in the February 1995 issue of _C++ Report_ magazine. Name the author of that article,
who has done much research on design patterns in C++ and other languages.

10. Given the function declaration `int *f();` (SAY: "int star eff open-paren close-paren semicolon")
at global scope, what is `decltype(f)` (SAY: "deckle type of eff")? Write your answer as a valid
C++ type that could appear to the right of the equals sign in an alias declaration.

----

If you missed the first game,
[a second game is happening Thursday evening at 4:00pm Aurora time](https://cppcon2020.sched.com/event/eU1G/c-pub-quiz).
Even if you're not virtually at CppCon already, there's still time for you to
[buy a reduced-price conference pass](https://cppcon2020.eventbrite.com?discount=TasteOf_CppCon_Attendee)
and get online in time to kick some butt at Pub Quiz!

Oh, and there's some great conference talks Thursday and Friday, too.

See also:

* ["C++ Pub Quiz at CppCon 2020, Game Two"](/blog/2020/09/18/cppcon-2020-pub-quiz-2/) (2020-09-18)
