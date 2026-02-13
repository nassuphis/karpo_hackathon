notes on paths.

n length of paths, how many there are
k is the specific path numer, which of all paths 
one path has vectors containing the path's coordinates:
    real vector of path's real values
    imag vector of path's imaginary values
    speed: what % is completed in 1 second

iteractive mode, input is seconds since "play" was pressed.
we need to convert seconds into index for a path

for k in paths
k.len = length of k-th path's real vector .. (imag and real same length!)
k.i  = int(speed * elapsed seconds * klen ) % klen  /* 0-based */

when will all paths be in sync? when all k.i are zero at the same time.
at 0 elapsed seconds, all k.i are 0, so in sync at start.
when again? 
if s = elapsed seconds 




