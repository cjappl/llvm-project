# Title

> Please use [AP style title case](https://titlecase.com/).
 Acronyms should be all uppercase. Code elements such as keywords, 
function names, etc that appear in your title should have the case they 
would have when written in code. Subtitles should be set off with 
colons, not dashes, using a space after, but not before the colon. See 
also [Boost library naming rules](https://www.boost.org/development/requirements.html#Naming_consistency). For example, "My Adventures With Boost.Asio: return to Sender"
> 

**ADC Title: RADSan: A Realtime-Safety Sanitizer**

## Options

- Catching Realtime Safety Violations at Compile and Runtime with New Tools in Clang
- New Realtime Safety Diagnostics in the LLVM Ecosystem
- Realtime Sanitizer and perf-constraints: Guaranteeing Realtime Safety with Clang
- Constraining Realtime Code to be nonblocking and nonallocating with Clang
- Detecting and Preventing Realtime Violations with LLVM's New Tools
- Enhancing Realtime Programming Safety in Clang: Runtime and Compile-time Solutions
- **LLVM's Realtime Safety Revolution: Tools for Modern Mission Critical Systems**
- From Detection to Prevention: Clang's Comprehensive Realtime Safety Tools

# Abstract

> As it should appear in the program. About one to three paragraphs. This is your pitch to both the Program Committee and to potential  attendees about why they should see your presentation. (Use the 'Comments' field below for remarks intended only for the PC.) 
>
> Try to answer the reader's questions, What will I learn? and Why is this important to me?
>
To facilitate a double-blind review process, please avoid statements that remove all uncertainty about who you are: Instead of: This presentation is a follow-on to , my talk last year. consider: This presentation builds on, from last year’s conference. It isn’t necessary to make it impossible to guess who you are, but please allow for some uncertainty.
> 
>
> Note: 
Conference organizers do not modify abstracts without a submitter’s consent, but we reserve the right to make appropriate edits for anonymization purposes.
> 
>
> Note also: After a submission has been accepted, presenters can edit their abstract as desired without this restriction.
> 

"ERROR: RealtimeSanitizer: call to malloc detected during execution of realtime function MyAudioCallback::process!"

"Warning: MyAudioCallback::process must not call blocking fuction ‘SkectchyCall`"

Realtime programmers working on mission-critical audio, autonomous vehicle, and aerospace code are well-acquainted with the golden rule: “Thou shalt not call time-unbounded operations in your realtime thread.” Despite its importance, tools to enforce this rule have been non-existent—until now!

In the latest version of Clang, two new features help uphold realtime guarantees by preventing `malloc`, system calls, and user-defined "unsafe" functions. First, we introduce the Realtime Sanitizer, which detects calls to `malloc`, `pthread_mutex_lock`, and other problematic functions in your realtime code at runtime. Next, we explore the `-Wperf-constraints` system, which provides similar feedback statically at compile time. We will compare and contrast these methods and offer recommendations on how to integrate them into your codebase effectively.

By leveraging these new tools, you can ensure your real-time systems remain robust, reliable, and ready for any challenge.

# Outline

> You'll not be held to this—we understand that this is a snapshot in time.
Feel free to include any comments that will help the Program Committee understand what you intend to present (and/or omit).
>This field will not appear in the final program.

- Introduction
    - Authors
- What is “Realtime”
    - Anything where code’s correctness depends on the answer being delivered in the right amount of time.
        - Audio callbacks
        - Aerospace guidance systems
        - Video game frame rendering
        - Low-latency programming (like HFT)
- What can and can’t be done in a realtime context?
    - `malloc`, `free`, `unlock`, `lock`
    - Anything that doesn’t have an obvious answer to “how long might this take, in the worst case?”
    - System calls, code that isn’t wait free (this thread may not make progress for some amount of time), many thread synchronization mechanisms.
- What are the consequences for realtime callback failure?
    - Audio glitches
        - It’s all fun and games until your DJ software drops out at 150dB and destroys eardrums and your speaker system
    - Frame drops in video games
    - Maybe your rocket blows up?
- These problems are hard to catch
    - Maybe all of us know what `malloc` looks like in code, even “incognito” in `std::unique_ptr` or `push_back`
    - What about `third_party_library::process_geometry`, is that realtime safe?
    - What about `push_back` that doesn’t always allocate, but only does when there isn’t enough space available?
- The stone age (2023). How were these problems avoided?
    - Avoidance techniques
        - Shared experience
        - Code review
        - Profilers and debuggers
        - static_assert
        - Documentation
        - Using your ears (or your eyes, to see if the rocket blew up)
    - However, these all have problems
        - Getting experience takes a **long time**
        - Code review is prone to **human error**
        - Profiling/debugging is a **manual process**
        - Static assertions are **limited**
        - Documentation **goes out of date**
        - Clicks and pops are **unlikely**
    - What about pre-built binaries?
        - Ouch, no real way to check those
- What if we could check for these errors at runtime? Or when your code was being compiled?
- Introducing the tools
    - Realtime Sanitizer
        - Check at runtime for any of these errors, including in pre-built binaries.
    - perf-constraints
        - Get compiler warnings and errors at compile time if you call any unsafe functions.

Realtime sanitizer

- How it works
    - High level overview of how sanitizers work in a general sense
    - Tour of the interceptors
    - What’s going on under the hood (very high level)
    - Tour of the appropriate `[[nonblocking]] [[blocking]]` attributes
    - Demo or screenshot showing running a binary and getting error output

Perf-constraints

- How it works
    - Tour of the attributes (nonblocking, nonallocating).
    - Show some of the errors that are generated.
	* Show some of the automatic things it catches, like `static` local variables.
    - Showing inference in the same translation unit.
    - Showing re-declaration if you don’t have access to the source, or it’s outside the compilation unit.

Comparing and contrasting

- Static vs runtime
    - Perf-constraints: If you code compiles without warnings, it is realtime violation free.
        - Recommendation: Use `Werror` !
    - RADSan: If you ran your code and it didn’t exit non-zero, that path is realtime violation free
        - Recommendation: Needs lots of test coverage, either unit or manual with the instrumented binary.
- What could happen versus what did happen
    - Perf-constraints: Shows “what could happen” in your realtime context. Every path through your nonblocking code only calls other nonblocking code.
    - Realtime sanitizer: Shows “what actually did happen”. This includes enforcing closed source compiled third party libraries never allocated during your run.
    - e.g. `push back`
        - If you have `reserve`d enough space, this is realtime safe. If you haven’t it is not.
        - `perf-constraints` would complain about this call, as it isn’t safe in all context
        - RADSan would only complain if an allocation actually does occur.
        - As a side note:
            - If you knew this `push_back` to always be safe, disable the warning with a `pragma`, more on this in a second
    - e.g. Something realtime unsafe happens in a case where your testing can’t feasibly get to
        - RADSan would never error in this case, you have a problem lurking that may never be detected.
        - Perf-constraints would tell you that this could happen, and you can determine if it’s necessary to fix!
- Converting a codebase “cost”
    - Perf-constraints: Every method called from a `nonblocking` or `nonallocating` function must also be appropriately attributed.
        - In large codebases, this may take some time.
        - In third party codebases, you may have to go digging through a lot of code to re-declare what calls are `nonblocking` or `nonallocating`.
        - Recommendation:
            - Scripting may be a solution.
            - Break the problem down in parts, wrap the code in warning-disabling pragmas and slowly unwrap when you check out the function. (more on this in a few slides)
    - Realtime sanitizer: Attribute your top level realtime callback and run.
        - Potential hidden cost in needing more test coverage (but isn’t that also a good thing?)
- Runtime cost
    - Perf-constraints: All done at compile time, no runtime cost
    - Realtime sanitizer: Will (mildly, one thread local conditional check) slow down system calls that are **not** in the realtime thread. Should not affect performance of the realtime threads when there are no realtime safety violations -- if you have nothing to hide, you have nothing to fear :)
- Disabling for code you know is safe
    - Perf-constraints: Disable the warning for a block with a `pragma`
        - This is a great way to incrementally start converting a codebase
    - Realtime sanitizer: `nosanitize("realtime")` on any functions that are “known safe”.
- Customization
    - Motivating example
        - Some heavily contested spin lock, or CAS loop that isn’t wait free, but isn't naturally detected by either system as it could be safe.
    - Both: Declare this function as `[[blocking]]` and both systems will pick it up.

Both tools working in harmony

- The great thing is you can have both! Both tools run on the same attribute, so starting out you can lean towards the Realtime sanitizer, and slowly start to integrate `perf-constraints` or vice versa!

Conclusion

- Don’t blow up your rocket, new clang tools are here to help!
