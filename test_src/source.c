bool first, isTrue = true, jefry;

int functionReturnC()
{
	int a = 100, c = 10;
	if (a > c) {
		c = 0;
	}
	return c;
}

int functionRetSecondArg(bool a, int cd)
{
	return cd;
}

int functionReturnMax(int first, int second) {
	if (first > second) {
		return first;
	}
	return second;
}

int functionReturnMax(int first, int second, int third) {
	int max = first;
	if (second > max) {
		max = second;
	}
	if (third > max) {
		max = third;
	}
	return max;
}

int main() // main func
{
	/* some stuff here
	and here
	*/
	int js = 3;
	bool s = true;
	int array[2][4];
	array[0][2] = 5;
	array[1][1] = 29;
	array[1][3] = 56;
	return functionReturnMax(array[1][1], array[0][2], array[1][3]);
}
