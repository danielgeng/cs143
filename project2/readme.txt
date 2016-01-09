Project 2: Implement B+Tree indexes for Bruinbase
http://oak.cs.ucla.edu/cs143/project/bruinbase/index.html

---
Design

Leaf node structure:
Number of keys | key | RecordId | key | RecordID | ... | PageId to next leaf

Non-leaf node structure:
Number of keys | PageId | key | PageId | key | PageId | ... etc ...

---
Testing

For the most part, the number of page reads is consistent with the expected output. All of the outputs are as expected.

xsmall 3, 3 (correct: 2-3, 2-3)
small 3, 7 (correct: 2-3, 6-7)
medium 5, 5 (correct: 4-5, 4-5)
large *20, *17, *17 (correct: 21-22, 15-16, 15-16)
xlarge 219, *7, 72 (correct: 219-220, 5-6, 69-73)
