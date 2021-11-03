#include "header.h"
#include "AVL_INDEX.h"

void print(DATA ss) {
	cout << ss.key << " " << ss.frequency << " times at :\n" ;
	for (auto x: ss.info) {
		cout << "(" << x.first << ", " << x.second << ") ";
	}
	cout << endl << endl;
}

int main() {
	AVL_INDEX<DATA, string> tree;
	
	if(tree.AVL_Empty()) 
		cout << "Empty tree."<< endl;
	while (true) {
		int choice = 0;
		cout << " 1. Build index from text file\n"
			 << " 2. Save index to text file\n"
			 << " 3. Load index from text file\n"
			 << " 4. Search\n"
			 << " 5. Trim low frequency\n"
			 << " 6. Print AVL tree\n"
			 << " 7. Print index\n"
			 << "-1. Exit\n\t";
		cin >> choice;

		switch(choice) {
			case 1:
				tree.AVL_buildIndex();
				break;
			case 2:
				tree.AVL_Save();
				break;
			case 3:
				tree.AVL_Load();
				break;
			case 4:
				tree.AVL_Search();
				break;
			case 5:
				tree.AVL_Trim();
				break;
			case 6:
				tree.AVL_Print();
				cout << endl;
				break;
			case 7:
				tree.AVL_Traverse(print);
				break;
			case -1:
				cout << "Exiting...\n";
				return false;
				break;
			default:
				cout << "Invalid input...\n";
				break;
		}
	}

    return 0;
}
