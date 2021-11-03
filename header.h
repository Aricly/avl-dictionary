#ifndef HEADER_H_
#define HEADER_H_

#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<iomanip>
#include<string.h>
#include<vector>
#include<utility>
#include<ctype.h>
#include<cstdio>
#include<queue>

using namespace std;

struct DATA
{
	string key;
	vector<pair<string, int>> info;
	int frequency;
	bool operator<(const DATA& rhs) const {
		return frequency > rhs.frequency;
	}
};

#endif /* HEADER_H_ */