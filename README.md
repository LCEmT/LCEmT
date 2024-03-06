# LCEmT
(Lossless Compression Techniques for Embedding Tables 
in Substantial Deep Learning-Based Recommendation System)[project_poster.pdf]

## Abstract
Deep learning-based recommendation systems are popular in a variety 
of fields. However, one of the most significant challenges is the
high spatial cost of embedding tables. Our research focuses on 
lossless compression for embedding tables, which can effectively reduce
spatial costs without sacrificing recommendation accuracy. We propose 
a new lossless compression method and compare the results of the
experiments with the theoretical limit of lossless compression.

## Introduction
Deep learning-based recommendation systems achieve high
accuracy but come with a significant spatial and computational
cost. One major contributor to spatial cost is embedding table,
which contains the information of users and items. As a result, the
compression for it becomes imperative.
Embedding table is a large matrix with typically large number
of rows, each of uniform length. A well-known lossless
compression method is Huffman coding. However, this approach
introduces non-uniform row lengths, requiring extra handling. To
ensure hardware efficiency, the compressed table must maintain
the feasibility of random access, namely, to keep the row interval
fixed. This limit the compression ratio to the longest row of table.
Directly split the Huffman-coded table row by row may result in
poor compression ratios, especially when longer codings share a
row. To address these, we propose new coding methods.

![image](https://github.com/LCEmT/LCEmT/assets/119176220/38e1b646-2aa1-4eb0-a8c1-b5246917014e)

## Methodology
Instead of directly building the coding tree, we first decide the
shape of the tree and then assign the characters to the tree based on
their frequency. The latter is a fixed algorithm; therefore, our main
goal is to find the optimal shape of the tree

### Construction of Coding Tree
In our project, the character length is 8-bit. As a result, the
coding tree will have 256 leaves. It is impossible to find the
optimal tree by enumeration of all structures. We propose two
methods to address this problem. The first method is to split a
character into two 4-bit characters. The second method is to use
simulated annealing algorithm to find the optimal 8-bit coding tree.

### Method 1: Split into two 4-bit
Converting 8-bit encoding to 4-bit encoding can enhance
decoding speed and find the tree with the highest compression
ratio by enumerating all structures. However, the compression
ratio may be sacrificed in this process. To achieve even higher
compression ratios, we employ two 4-bit trees, dividing the data
into two sets and allocating them using a greedy algorithm.
The steps of the algorithm are roughly as follows. First, we
identify the globally optimal tree by enumeration. Then, we
exhaustively compress with the second tree starting from the row
with the smallest compression ratio until there was no further
improvement.
![image](https://github.com/LCEmT/LCEmT/assets/119176220/2e9c6fa8-9b23-41ae-bdae-f124cb11221e)


### Method 2: Simulated Annealing for 8-Bit Coding Tree
In finding an optimal 8-bit coding tree, we approach it as an
optimization problem and using simulated annealing algorithm
to find the best coding tree.
The process begins with the setup of starting coding tree based on the
data, followed by multiple iterations. Each iteration fine-tune the coding
tree to explore neighboring structures. Then decide whether to accept the
new coding tree based on the resultant compression ratio and
annealing temperature, which is the parameter governing the willingness
to accept new outcomes. By scheduling these parameters, the
coding tree will converge toward optimal gradually.
For the starting coding tree, we tried two types: the Huffman
tree and the adjusted balanced tree. Given that not all characters
necessarily appear in the data, we can prune unnecessary leaves
to enhance the adjusted balanced tree

![image](https://github.com/LCEmT/LCEmT/assets/119176220/be4b294b-3a55-4694-90e9-77af68aca1c8)
![image](https://github.com/LCEmT/LCEmT/assets/119176220/7f6c9764-8a5e-4a91-948d-733fda8598cb)


### Theoretical Limit
To find the theoretical limit of lossless
compression, we perform 4-bit or 8-bit
Huffman encoding individually for each row.
At this stage, each row is the optimal
compressed result. The row with the smallest
compression ratio stands as the theoretical
upper limit for these 4-bit or 8-bit data.

![image](https://github.com/LCEmT/LCEmT/assets/119176220/e465b912-2bd0-4407-bd9b-435b05eb9b76)

## Results
![image](https://github.com/LCEmT/LCEmT/assets/119176220/363060e3-6dd7-41bc-a88e-0f5aae6375aa)

## Conclusion
Our approaches achieve a compression ratio close to the
theoretical limit. For Method 1, the average difference from the
limit is 0.36%. For Method 2, the average difference from the limit
is 5.78%. Excluding DLRM, the average difference from the limit
would be 1.09%.
In conclusion, we have introduced a novel compression
algorithm that exhibits commendable performance and is well-suited 
for hardware implementation.
