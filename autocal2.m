
fn = ['recdata-', cid, '.txt'];

total = 0;
total2 = 0;

re = [1, 1, 1, 1, 1, 1
      1, 1, 1, 1, 1, 1];

fid = fopen(fn);
[A, cnt] = fscanf(fid, '%x', [26, 400]);

for cn=1:12

low	= A(2 * cn - 1,:);
high = A(2 * cn,:);

[k, b, k2, b2, c, h, cerr, cerr2] = calibcol(high, low);

total = total + cerr;
total2 = total2 + cerr2;

re(cn, :) = [k,b,k2,b2,c,h];

end

ofn = ['kbmat-', cid, '.dat'];

%csvwrite(ofn, re)

dlmwrite(ofn, re, 'newline', 'pc');

% type ofn

total
total2


