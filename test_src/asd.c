bool f, s = true, j;

int func()
{
	int a = 100, c = 10;
	if (a > c) {
		c = 0;
	}
	return c;
}

int func(bool a, int cd)
{
	return cd;
}

int func(bool a) {
	return 99;
}

int main()
{
	int js = 3;
	return func(true, 42);
}
