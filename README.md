# 2023Spring Operating Systems --- Lab 2: Shell

## Introduction
The **shell** is the main command-line interface between a user and the operating system, and it is an essential part of the daily lives of computer scientists, software engineers, system administrators, and such. 
It makes heavy use of many OS features. 
In this lab, you will build a simplified version of the Unix shell called the **New Yet Usable SHell**, or `nyush` for short.

## Objectives
Through this lab, you will:

* Familiarize yourself with the Linux programming environment and the shell, of course.
* Learn how to write an interactive command-line program.
* Learn how processes are created, destroyed, and managed.
* Learn how to handle signals and I/O redirection.
* Get a better understanding of the OS and system calls.
* Be a better C programmer and be better prepared for your future technical job interviews. In particular, the string parsing skill that you will practice in this lab is desired in many interview questions.

## Overview
The shell is essentially a command-line interpreter. It works as follows:

1. It prompts you to enter a command.
2. It interprets the command you entered.
* If you entered a built-in command (e.g., cd), then the shell runs that command.
* If you entered an external program (e.g., /bin/ls), or multiple programs connected through pipes (e.g., ls -l | less), then the shell creates child processes, executes these programs, and waits for all these processes to either terminate or be suspended.
* If you entered something wrong, then the shell prints an error message.
3. Rinse and repeat until you press Ctrl-D to close STDIN or enter the built-in command exit, at which point the shell exits.

## Setup
1. Clone this repository in your computer
2. Inside the `nyush` folder terminal, run `make` to compile and create an executable file
3. run `./nyush`

Lab Website: https://cs.nyu.edu/courses/spring23/CSCI-GA.2250-002/nyush#theprompt
