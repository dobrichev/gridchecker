#pragma once

struct morph {
	int transpose; //0..1
	int stackPerm;	//0..5
	int stack1ColPerm;	//0..5
	int stack2ColPerm;	//0..5
	int stack3ColPerm;	//0..5
	int bandPerm;	//0..5
	int band1RowPerm;	//0..5
	int band2RowPerm;	//0..5
	int band3RowPerm;	//0..5
	void byIndex(int i) {
		band3RowPerm = i % 6; i /= 6;
		band2RowPerm = i % 6; i /= 6;
		band1RowPerm = i % 6; i /= 6;
		bandPerm = i % 6; i /= 6;
		stack3ColPerm = i % 6; i /= 6;
		stack2ColPerm = i % 6; i /= 6;
		stack1ColPerm = i % 6; i /= 6;
		stackPerm = i % 6; i /= 6;
		transpose = i % 2;
	}
	int index() const {
		int i = transpose;
		i *= 6; i += stackPerm;
		i *= 6; i += stack1ColPerm;
		i *= 6; i += stack2ColPerm;
		i *= 6; i += stack3ColPerm;
		i *= 6; i += bandPerm;
		i *= 6; i += band1RowPerm;
		i *= 6; i += band2RowPerm;
		i *= 6; i += band3RowPerm;
		return i;
	}
};

class allTranformations {
	ch81 * indices; //where to go
	static int allCallback(void *context, const char *puz, char *m, const morph &theMorph);
public:
	static const int count;
	allTranformations();
	~allTranformations();
	void transform(const ch81 &src, ch81 &dest, int transformationIndex) const;
};
