#include "header.h"
#include "AVL_ADT.h"

template <class TYPE, class KTYPE>
class AVL_INDEX: public AvlTree <TYPE, KTYPE> {
    private:
        bool _update    (TYPE item);
        void _search    (string search_key, priority_queue<TYPE>& search_terms, NODE<TYPE> *root);
        /*float _ptw      (int frequency, int count);*/                               /* was used to calculate per thousand words, could not find a good ratio, upto 0.00000001 */
        void _trim      (NODE<TYPE> *root, int ratio, vector<KTYPE>& to_del);
        void _destroyAVL (NODE<TYPE>  *root);                                                   /* modified to free vector memory with swap to temp vector */
        NODE<TYPE>* _delete(NODE<TYPE> *root, KTYPE dltKey, bool& shorter, bool& success);      /* modified to free vector memory with swap to temp vector */

    public:
        bool  AVL_Insert   (TYPE   dataIn);
		bool  AVL_buildIndex();
        void  AVL_Save();
        void  AVL_Load();
        void  AVL_Search();
        void  AVL_Trim();

};

template <class TYPE, class KTYPE>
bool AVL_INDEX<TYPE, KTYPE> :: _update (TYPE item) {
    NODE<TYPE> *newPtr = this->_retrieve(item.key, this->tree);
    if (newPtr) {
        for (auto x: newPtr->data.info) {
            if ((strcmp(item.info.back().first.c_str(), x.first.c_str()) == 0) && (item.info.back().second == x.second)) 
                return true;
        }
        newPtr->data.info.push_back(make_pair(item.info.back().first, item.info.back().second));
        newPtr->data.frequency = newPtr->data.info.size();
        return true;
    }

    return false;
}

template <class TYPE, class KTYPE>
bool AVL_INDEX<TYPE, KTYPE> :: AVL_Insert (TYPE dataIn) 
{
//	Local Definitions 
	NODE<TYPE>  *newPtr;
	bool         taller;

//	Statements 
    if (!_update(dataIn)) {
        if (!(newPtr = new NODE<TYPE>))
        	return false;
        newPtr->bal    = EH;
        newPtr->right  = NULL;
        newPtr->left   = NULL;
		newPtr->data.key = dataIn.key;
        newPtr->data.info.push_back(make_pair(dataIn.info.back().first, dataIn.info.back().second));
        newPtr->data.frequency = newPtr->data.info.size();

        this->tree = this->_insert(this->tree, newPtr, taller);
        this->count++;
    }
	return true;
}

template <class TYPE, class KTYPE>
bool AVL_INDEX<TYPE, KTYPE> :: AVL_buildIndex() {
	char buf[100], word[200];
    DATA tempItem;
    memset(word, 0, sizeof word);	/* empty word[] */

    while (true) {
        int index = 0;
        int wordCount = 0;

        cout << "Enter file name(q! to quit): ";
        cin >> buf;

        if (buf[0] == 'q' && buf[1] == '!') {
            cout << "Returning...\n\n";
            return false;
        }   /* Checks for exit statement */
        FILE *fp;
        if (( fp= fopen(buf, "r")) == NULL) {
            perror("Invalid File!");
            continue;
        }   /* Checks file/directory existence */

        string strn = buf;
        while (!feof(fp)) {
            char c = fgetc(fp);

            if (isalpha(c) && islower(c) == 0)
                c = tolower(c);
            /* Check for uppercase and convert to lowercase */
            if (c == ' ' || c == '\n' || c == '\t' ||  c == '\0') {
                if (!isalpha(word[0]))
                    continue;
                index = 0;
                wordCount++;
                string tempWord = word;
                tempItem.key = tempWord;
                tempItem.info.push_back(make_pair(strn, wordCount));
                this->AVL_Insert(tempItem);
            /* Check for new words */
                memset(word, 0, sizeof word);	/* empty word[] */
            } else {
                if (!iswalnum(c))
                    continue;
                word[index] = c;
                index++;
            }
        }
        //this->AVL_Delete("");
        cout << "Index has been built using " << buf << endl;
        fclose(fp);
    }

	return true;
}

template <class TYPE, class KTYPE>
void AVL_INDEX<TYPE, KTYPE> :: AVL_Save() {
    char buf[100];
    FILE *fp;

    while (true) {
        cout << "Enter Save File(q! to quit): ";
        cin >> buf;

        if (buf[0] == 'q' && buf[1] == '!') {
            cout << "Returning...\n\n";
            return;
        }   /* Checks for exit statement */

        
        if (( fp= fopen(buf, "w")) == NULL) {
            perror("File Creation");
            continue;
        }   /* Checks file create status */

        vector<NODE<TYPE> *> level_hold;
        level_hold.insert(level_hold.begin(), this->tree);
        /* add root of tree to list */
        while (!level_hold.empty()) {
            NODE<TYPE> *node = level_hold.back();
            level_hold.pop_back();
            fprintf(fp, "%s", node->data.key.c_str());
            putc('`', fp);
            fprintf(fp, "%d", node->data.frequency);
            putc(' ', fp);
            /* print to file, key frequency eg. word 4 */
            for (auto x: node->data.info) {
                fprintf(fp, "%s", x.first.c_str());
                putc('*', fp);
                fprintf(fp, "%d", x.second);
                putc('&', fp);
            }
            /* print to file, location(s) eg. text1.txt*51 */
            putc('\n',  fp);
            
            if (node->left != NULL)
                level_hold.insert(level_hold.begin(), node->left);
            if (node->right != NULL)
                level_hold.insert(level_hold.begin(), node->right);

        }
        cout << "AVL Tree has been saved to: " << buf << endl << endl;
        fclose(fp);
    }
    
}

template <class TYPE, class KTYPE>
void AVL_INDEX<TYPE, KTYPE> :: AVL_Load() {
    char buf[100], word[200];
    int index;  
    vector<pair<string, int>> location;
    string str, fname;
    FILE *fp;
    memset(word, 0, sizeof word);	/* empty word[] */

    while (true) {
        cout << "Enter Load File(q! to quit): ";
        cin >> buf;

        if (buf[0] == 'q' && buf[1] == '!') {
            cout << "Returning...\n\n";
            return;
        }   /* Checks for exit statement */

        
        if (( fp= fopen(buf, "r")) == NULL) {
            perror("File Opening");
            return;
        }   /* Checks file create status */

        NODE<TYPE> *newItem;
        if (!(newItem = new NODE<TYPE>))
        	perror("Load Error");
        newItem->bal = EH;
        newItem->left = NULL;
        newItem->right = NULL;
        /* Layout of save is
            Key`Frequency File*Wordcount&File*Wordcount...
         */
        while (!feof(fp)) {
            bool taller = false;
            char c = fgetc(fp);
            
            if (c == '\n') {        /* If \n which indicates a new node is starting, insert current node into tree */
                newItem->data.info = location;
                if (!_update(newItem->data)) {
                    this->tree = this->_insert(this->tree, newItem, taller);
                    this->count++;
                }
                memset(word, 0, sizeof word);	/* empty word[] */
                index = 0;
                location.clear();
                newItem = new NODE<TYPE>;
                newItem->bal = EH;
                newItem->left = NULL;
                newItem->right = NULL;
                continue;
            } else if (c == '`') {      /* If ` which indicates key, set key */
                str = word;
                newItem->data.key = str;

                memset(word, 0, sizeof word);	/* empty word[] */
                index = 0;
                continue;
            } else if (c == ' ') {      /* If space which indicates frequency, set frequency */
                newItem->data.frequency = atoi(word);

                memset(word, 0, sizeof word);	/* empty word[] */
                index = 0;
                continue;
            } else if (c == '*') {      /* If * which indicates location of file, set name */
                fname = word;

                memset(word, 0, sizeof word);	/* empty word[] */
                index = 0;
                continue;
            } else if (c == '&') {      /* If & which indicates the word location in the file, set location and push into vector */
                location.push_back(make_pair(fname, atoi(word)));

                memset(word, 0, sizeof word);	/* empty word[] */
                index = 0;
                continue;
            }

            word[index] = c;
            index++;
        }
        
        cout << "AVL Tree has been loaded from: " << buf << endl << endl;
        fclose(fp);
    }
}

template <class TYPE, class KTYPE>
void AVL_INDEX<TYPE, KTYPE> :: AVL_Search() {
    char buf[100];
    string str;
    priority_queue<TYPE> terms;

    while (true) {
        cout << "Enter search term(q! to quit): ";
        cin >> buf;

        if (buf[0] == 'q' && buf[1] == '!') {
            cout << "Returning...\n\n";
            return;
        }   /* Checks for exit statement */
        str = buf;

        _search(str, terms, this->tree);

        vector<TYPE> temp_list;
        while (!terms.empty()) {
            auto temp = terms.top();
            temp_list.push_back(temp);
            terms.pop();
        }
        int i = 0;
        if (temp_list.empty()) {
            cout << "Nothing found..\n\n";
            continue;
        }
        cout << "Term '" << str << "' found at: \n";
        for (auto x: temp_list)
            cout << ++i << ". " << x.key << " " << x.frequency << " times\n";
        cout << "\nChoose to see locations: ";
        char c;
        cin >> c;
        if (i < 1 || i > temp_list.size() || !isdigit(c)) {
            cout << "Invalid size\n\n";
        } else {
            i = c - '0'; 
            i--;
            cout << temp_list[i].key << " " << temp_list[i].frequency << " times at: ";
            for (auto j: temp_list[i].info)
                cout << "(" << j.first << ", " << j.second << ") ";
            cout << endl << endl;
        }

        vector<TYPE>().swap(temp_list);
        memset(buf, 0, sizeof buf);
    }
}

template <class TYPE, class KTYPE>
void AVL_INDEX<TYPE, KTYPE> :: _search(string search_key, priority_queue<TYPE>& search_terms, NODE<TYPE> *root) {
    if (search_terms.size() > 5) {
            search_terms.pop();
        }
    if (root) {
        
        if (strstr(root->data.key.c_str(), search_key.c_str()) != NULL) {
            search_terms.push(root->data);

        }
        _search(search_key, search_terms, root->left);
        _search(search_key, search_terms, root->right);
    } else return;
}

/*
template <class TYPE, class KTYPE>
float AVL_INDEX<TYPE, KTYPE> :: _ptw(int frequency, int count) {
    return frequency/count*1000;
}
*/
template <class TYPE, class KTYPE>
void AVL_INDEX<TYPE, KTYPE> :: AVL_Trim() {
    int ratio;
    vector<KTYPE> to_del;
    cout << "Enter desired cut-off frequency: ";
    cin >> ratio;
    _trim(this->tree, ratio, to_del);
    for(auto x: to_del)
        this->AVL_Delete(x);
    cout << "Tree has trimmed words with less than " << ratio << " occurrences\n";
}

template <class TYPE, class KTYPE>
void AVL_INDEX<TYPE, KTYPE> :: _trim(NODE<TYPE> *root, int ratio, vector<KTYPE>& to_del) {
    if (root) {
        if (root->data.frequency < ratio)
            to_del.push_back(root->data.key);
        _trim(root->left, ratio, to_del);
        _trim(root->right, ratio, to_del);
    } else return;
}

template <class TYPE, class KTYPE>
void AVL_INDEX<TYPE, KTYPE> :: _destroyAVL(NODE<TYPE> *root) {
	if (root)
	   {
	    _destroyAVL (root->left);
	    _destroyAVL (root->right);
        vector<pair<string, int>>().swap(root.data.info);
	    delete root;
	   }
	return;
}

template <class TYPE, class KTYPE>
NODE<TYPE>* AVL_INDEX<TYPE,  KTYPE>:: _delete(NODE<TYPE> *root, KTYPE dltKey, bool& shorter, bool& success) {
//  Local Definitions 
	NODE<TYPE> *dltPtr;
	NODE<TYPE> *exchPtr;
	NODE<TYPE> *newRoot;

// 	Statements 
	if (!root)
	   {
	    shorter = false;
	    success = false;
	    return NULL;
	   } //  if -- base case 
	
	if (dltKey < root->data.key)
	    {
	     root->left = _delete (root->left, dltKey, 
	                           shorter,    success);
	     if (shorter)
	         root   = dltRightBalance (root, shorter);
	    } // if less 
	else if (dltKey > root->data.key)
	    {
	     root->right = _delete (root->right, dltKey,
	                            shorter,     success);
	     if (shorter)
	         root = dltLeftBalance (root, shorter);
	    } //  if greater 
	else
	    //  Found equal node 
	    {
	     dltPtr  = root;
	     if (!root->right)
	         // Only left subtree 
	         {
	          newRoot  = root->left;
	          success  = true;
	          shorter  = true;
              vector<pair<string, int>>().swap(dltPtr.data.info);
	          delete (dltPtr);
	          return newRoot;            //  base case 
	         } //  if true 
	     else
	         if (!root->left)
	             //  Only right subtree 
	             {
	              newRoot  = root->right;
	              success  = true;
	              shorter  = true;
                  vector<pair<string, int>>().swap(dltPtr.data.info);
	              delete (dltPtr);
	              return newRoot;        // base case 
	            } //  if 
	         else
	             //  Delete NODE has two subtrees 
	             {
	              exchPtr = root->left;
	              while (exchPtr->right)
	                  exchPtr = exchPtr->right;
	                  
	              root->data = exchPtr->data;
	              root->left = _delete (root->left, 
	                                    exchPtr->data.key,
	                                    shorter, 
	                                    success); 
	              if (shorter)
	                  root = dltRightBalance (root, shorter); 
	             } //  else 
	
	    } // equal node 
	return root; 
}

