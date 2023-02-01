function [result, offset, ecode] = auto_correcti(hb, lb, ki1, bi1, ki2, bi2, ci, hi)

int_max = int32(intmax('int16'));

int_max256 = int_max / 256;

hbln = mod(hb, 16);
hbhn = idivide(int32(hb), 16);

if hb < 16
 	ki = ki1;
	bi = bi1 + hi / 2;        % the first hump is sometimes higher
	ci = ci + hi / 10;
elseif hb < 128 
	ki = ki1;
	bi = bi1;
else
	ki = ki2;
	bi = bi2;
end

offset = ki * hb / int_max + bi / int_max256;

lbc = lb + offset;

if hb > 128
	hbi = 128 + (hb - 128) / 2;
else
	hbi=hb;
end

offset = offset + ((lbc - 128) * ci * (300 - hbi)) / (12 * 300 * int_max256);    

% Since we are using lbc instead of hb we have to consider "lbc" has a shrink factor

lbc = lb + offset;

if lbc > 255
	lbc = 255;
elseif lbc < 0
	lbc = 0;
end

lbp = hbln * 16 + 7;
lbpc = lbp - offset;
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
	result = hbhn * 256 + lbc;
end

end