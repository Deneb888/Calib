function [result, offset, ecode] = auto_correct(hb, lb, k1, b1, k2, b2, c, shrink, hump)

hbln = mod(hb, 16);
hbhn = idivide(int16(hb), 16);

shrink = c * 0.0033;

if hb < 16
 	k = k1;
	b = b1 + 0.5 * hump;        % the first hump is sometimes higher
	c = c + 0.1 * hump;
elseif hb < 128 
	k = k1;
	b = b1;
else
	k = k2;
	b = b2;
end

offset = k * hb + b;

lbc = lb + offset;

if hb > 128
	hbi = 128 + (hb - 128) / 2;
else
	hbi=hb;
end

offset = offset + (lbc - 128) * (c - shrink * hbi) / 12;    

% Since we are using lbc instead of hb we have to consider "lbc" has a shrink factor

lbc = lb + offset;

if lbc > 255
	lbc = 255;
elseif lbc < 0
	lbc = 0;
end

lbp = hbln * 16 + 7;
lbpc = lbp - int16(offset);
qerr = lbp - lbc;

if lbpc > 255+20
	ecode = 1;
elseif lbpc > 255 && qerr > 28
	ecode = 2;
elseif lbpc > 191 && qerr > 52
	ecode = 3;
elseif qerr > 96
	ecode = 4;
elseif lbpc < -20
	ecode = 5;
elseif lbpc < 0 && qerr < -28
	ecode = 6;
elseif lbpc < 64 && qerr < -52
	ecode = 7;
elseif qerr < -96
	ecode = 8;
else
	ecode = 0;
end

if ecode > 0
	result = hb * 16 + 7;
else
	result = hbhn * 256 + int16(lbc);
end

end