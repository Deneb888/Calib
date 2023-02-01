fid = fopen(fn);
[A, cnt] = fscanf(fid, '%x', [26, 400]);

low	= A(2 * cn - 1,:);
high = A(2 * cn,:);

mod_h = mod(high, 16);
shifted = mod_h * 16 + 7;
diff = shifted - low;

cerr = sqrt(mean(diff .^2))

x = high;
y = diff;

plot(high, diff, 'o');

hold on

p = polyfit(x, y, 1);
y1 = polyval(p, x);
plot(x, y1-80, x, y1+80);

hold off

disp("hit a key...");
pause;

lb = y1-80;
hb = y1+80;

%=============poly fit with exclude===============

p1 = [1];
p2 = [2];

hold on

if high(400) < 208 + 3

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
		if x(i) >= startp && x(i) < endp && y(i) > lb(i) && y(i) < hb(i)
			k = k + 1;
			xx(k) = x(i);
			yy(k) = y(i);
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

%shrink = 0.02484;

%cc = p1(2) + 24*shrink - kk;

oo = p1(2:8);
p = polyfit(mm, oo, 1);

cc = p(2) - kk;
shrink = -p(1);

if cc > 5
  cc = 5;
  shrink = 0.0033 * cc;
end

%==============

mm = m(9:limit);
nn = n(9:limit);


p = polyfit(mm, nn, 1);

kk2 = p(1);
bb2 = p(2);

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

for i=1:400
	[result(i), offset(i), ecode(i)] = auto_correct(high(i), low(i), kk, bb, kk2, bb2, cc, shrink, 0);

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


for i=1:100
	if high(i) < 16
		[result(i), offset(i), ecode(i)] = auto_correct(high(i), low(i), kk, bb, kk2, bb2, cc, shrink, hump);

		if ecode(i) > 0  || high(i) > 247
			comp(i) = 7;
		else
			comp(i) = diff(i) - offset(i);
		end

	end
end

cerr2 = sqrt(mean(comp .^2))


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

plot(high, diff, 'o', high, offset, high, ecode*5);

cerr2 = sqrt(mean(comp .^2))

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




