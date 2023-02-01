fn = ['recdata-', cid, '.txt'];

fid = fopen(fn);
[A, cnt] = fscanf(fid, '%x', [26, 400]);

low	= A(2 * cn - 1,:);
high = A(2 * cn,:);

mod_h = mod(high, 16);
shifted = mod_h * 16 + 7;
diff = shifted - low;

cerr = sqrt(mean(diff .^2))

k = 1;
x = [1];
y = [1];

for i = 1:400
	if high(i) < 240 
		x(k) = high(i);
		y(k) = diff(i);
		k = k + 1;
	end
end

plot(high, diff, 'o');

hold on

p = polyfit(x, y, 1);
y1 = polyval(p, high);

lb = y1 - 70;
hb = y1 + 70;

plot(high, lb, high, hb);

hold off

disp("hit a key...");
pause;

%=============poly fit with exclude===============

p1 = [1];
p2 = [1];

hold on

if high(400) < 208 + 3			% What if high(400) is even smaller.

	limit=13;

elseif high(400) < 224 + 3

	limit=14;

else

	limit=15;

end

for j=2:limit

	xx = [1];
	yy = [1];
	k = 0;

	startp = (j-1) * 16;
	endp = j * 16;

	for i=1:400
		if high(i) >= startp && high(i) < endp && diff(i) > lb(i) && diff(i) < hb(i)
			k = k + 1;
			xx(k) = high(i);
			yy(k) = diff(i);
		end
	end

	p = polyfit(xx, yy, 1);
	yy1 = polyval(p, xx);

	plot(xx, yy, 'o', xx, yy1);

	p1(j) = p(1);
	p2(j) = p(2);

end

disp("hit a key...");
	pause;

%=========================

for i=1:limit
m(i) = 16 * i - 8;
end

for i=1:limit
n(i) = p1(i) * m(i) + p2(i);
end

mm = m(2:8);
nn = n(2:8);
p = polyfit(mm, nn, 1);

kk = p(1);
bb = p(2);

oo = p1(2:8);
p = polyfit(mm, oo, 1);

cc = p(2) - kk;
shrink = -p(1);

if cc > 6
  cc = 6;
  shrink = 0.0033 * cc;
end

%==============

mm = m(9:limit);
nn = n(9:limit);

p = polyfit(mm, nn, 1);

kk2 = p(1);
bb2 = p(2);

%=============== Below is for show only=================

r = linspace(1, 256, 256);
s = linspace(1, 256, 256);
ss = s;

s(1:128) = r(1:128) * kk + bb;
s(129:256) = r(129:256) * kk2 + bb2;

for i=1:128
	ss(i) = (mod(int16(r(i)), 16) - 8) * (cc - i * shrink);
end

for i=129:256
	ss(i) = (mod(int16(r(i)), 16) - 8) * (cc - 128 * shrink - (i - 128) * shrink * 0.5);
end

hold off
clf

plot(high,diff,'o', r, ss + s);

disp("hit a key...");
pause;

%=======================================================

int_max = int32(intmax('int16'));

int_max256 = int_max / 256;

ki1 = int32(kk * int_max)
ki2 = int32(kk2 * int_max)
bi1 = int32(bb * int_max / 256)
bi2 = int32(bb2 * int_max / 256)
ci = int32(cc * int_max / 256)
%hi = int32(hump * int_max / 256);


for i=1:400
	[result(i), offset(i), ecode(i)] = auto_correcti(high(i), low(i), ki1, bi1, ki2, bi2, ci, 0);

	if ecode(i) > 0 || high(i) > 247
		comp(i) = 7;
	else
		comp(i) = diff(i) - offset(i);
	end
end

%s=0;
%for i=1:400
%	s=s+comp(i)*comp(i);
%end

%cerr2 = sqrt(s/400)


%%%%%%%%%%%%%%%%hump%%%%%%%%%%%%%%%%%%%%

avg = 0;
cnt = 0;

for i=1:200
if high(i) < 16
	avg = avg + comp(i);
	cnt = cnt + 1;
end
end

if cnt > 0
	hump = avg / cnt
else 
	hump = 0
end

hi = int32(hump * int_max / 256)


for i=1:200
	if high(i) < 16
		[result(i), offset(i), ecode(i)] = auto_correcti(high(i), low(i), ki1, bi1, ki2, bi2, ci, hi);

		if ecode(i) > 0  || high(i) > 247
			comp(i) = 7;
		else
			comp(i) = diff(i) - offset(i);
		end

	end
end

cerr2 = sqrt(mean(comp .^2))

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

plot(high, diff, 'o', high, offset, high, ecode * 5);

for i=1:400
	spc(i) = ' ';
end

for i=1:400
	if ecode(i) > 0
		mark(i) = '*';
	else 
		mark(i) = ' ';
	end
end

disp("hit a key...");
pause;

final_view = [dec2hex(high), spc', dec2hex(low), spc', dec2hex(result), mark'];





