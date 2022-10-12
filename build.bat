@echo off

g++ -o main.exe .\src\main.cpp
type .\tests\test0.txt | .\main.exe