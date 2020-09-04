# xt
Execution trace tool

Hi all!

I am ashamed to say that this is my first project here on github.  I have been designing, building and programming computers since 1970... what else could I do?  My parents never bought me a dog.  Anyhow, here goes...

This is a small project that started when I was working on a completely different project.

I always thought it would be cool to have a small simple way to present a function call tree for my projects.  A number of large complex tools already exist (think Callgrind), but when you are working on a project you don't want to be distracted by learning something new... but then that's exactly what happened anyway.

As I said, this started as a spinoff of something completely different, but it's early origins were much different than this project.  It became clear as things moved on that a totally different approach was needed as the original plan imposed too many restrictions on how I worked.  It soon became clear that an essential feature was that any function execution tracing tool would have to be simple (drop in/drop out) and almost invisible to the user.  What appears here is the result of those requirements.

The details of the project are presented in the "Execution Trace Tool.docx" file, and the two main files, xt.c and xt.h are hopefully well enough documented to be useful.  Also provided is a simple make file that should simplify the use of the tool by allowing the user to do the  dropin/out directly from the make command line.  No other interaction should be needed.

I hope someone finds it useful.  You are free to use it, modify it, hack it, distribute it or eat it... just don't blame me if you get heartburn.
