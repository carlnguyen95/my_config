#!/usr/bin/python

######################################################################
# Write a Python function that takes a sequence of numbers and 
# determines whether all the numbers are different from each other. 
######################################################################

from array import *

def is_unique_arr(arr):
    set_arr = set(arr)
    if (len(set_arr) == len(arr)):
        return True
    else:
        return False

number_arr = list(map(int, input("Input the sequence of numbers seperated by space: ").split()))
print(number_arr)

if is_unique_arr(number_arr):
    print("The sequence of numbers is unique")
else:
    print("The sequence of numbers is not unique")
