#!/usr/bin/python

######################################################################
# Write a Python program that creates all possible strings using the 
# letters 'a', 'e', 'i', 'o', and 'I'. Ensure that each character is 
# used only once.
######################################################################

from re import sub


char_arr = ['a', 'e', 'i', 'o', 'I']

def shuffle_str(arr):
    if (len(arr) == 1):
        return arr.index(0)

i = 0
for char in char_arr:
    str_char = char
    for sub_char in char_arr:
        if (sub_char == char):
            pass
        else:
            pass

