#ifndef JSONLIB
#define JSONLIB 1

#include "std_lib_facilities.h"
#include "variables.h"

#define PROGRAMM_BEGIN int main(){
#define PROGRAMM_END ;return 0; }


#define PRINT_VARS  ;cout<<"vars:\n";							\
					for (item *i:vars){ cout<<*i; cout<<endl; }

vector<item*> vars;
/*void erase(item* i){
	item* parent = nullptr;
	for (item *it : vars){
		_erase(it, i, parent);
	}
		
}

void _erase(item* it , item* i , item* parent){
	static int inAr = 0;
	if (it == i){
		if (it->gettype() == OBJECT_N || it->gettype() == ARRAY_N){
			i->obj_val.clear();
			i->ar_val.clear();
		}
		else if (parent == nullptr){
			cout << "Wrong use of erase, is used only for array's items or object's items\n";
		}
		else{
			//....
		}
	}
	else if (i->gettype() == ARRAY_N){
		inAr++;
		parent = it;
		for (item it : i->ar_val){ _erase(&it ,i ,parent); }
		inAr--;
	}
	else if (i->gettype() == OBJECT_N){
		for (item it : i->obj_val){ }
	}
	else{
		cout << " UNKNOWN_TYPE ";
	}
}
*/
int isKey = 0;
string tmp_name;
vector<string> tmp_key;
string pop_key(){ string name = tmp_key.at(tmp_key.size() - 1); tmp_key.pop_back(); return name; }
struct{
	item& operator=(item &v){
		if (isKey == 0){
			vars.push_back(&v);
			return v;
		}
		v.set_name(pop_key());
		isKey = 0;
		return v;
	}
}tmp_k;


#define JSON(p) ;tmp_append=0; tmp_name=#p ; item p;tmp_k=p

#define NULL (*(new item()))
#define ARRAY (*(new item(ARRAY_N , tmp_name ,-1)))
#define TRUE ( *(new item( BOOL_N , tmp_name , 1) ))
#define FALSE ( *(new item( BOOL_N , tmp_name , 0) ))

#define NUMBER(i) ( *(new item( tmp_name ,i )) )
#define STRING(str) ( *(new item( tmp_name , str )) )
vector<item&> v;
#define OBJECT ( *(new item(OBJECT_N , tmp_name , -1 )) )+=v

#define KEY(key) (tmp_key.push_back(#key), isKey=1,											\
		    	 tmp_k) = 0 ? (cout << "something wrong happend", *new item("","") )

//variables processing.......................................................
#define SET ;tmp_append=0;
#define ASSIGN =
#define APPEND -=
#define ERASE ;tmp_append=0; --
//functions..................................
item get_size(item &i){
	if (i.gettype() == OBJECT_N){
		return *new item("" , i.Object::size());
	}
	else if (i.gettype() == ARRAY_N){
		return *new item("", i.Array::size() );
	}
	return *new item("",1);
}
item isEmpty(item &i){
	if (i.gettype() == ARRAY_N)
		return i.Array::isEmpty();
	if (i.gettype() == OBJECT_N)
		return i.Object::isEmpty();
	return *new item(BOOL_N, "", 0);
}
item has_key(item o , string key){
	if (o.gettype() != OBJECT_N){
		return *new item(BOOL_N , "" , 0);
	}
	item *ptr = o.find(key);
	if (ptr==nullptr)
		return *new item(BOOL_N, "", 0);
	return *new item(BOOL_N, "", 1);
}
string typeof(item i){
	if (i.gettype() == NULL_N)
		return "NULL";
	else if (i.gettype() == BOOL_N)
		return "boolean";
	else if (i.gettype() == NUM_N)
		return "number";
	else if (i.gettype() == STR_N)
		return "string";
	else if (i.gettype() == ARRAY_N)
		return "array";
	else if (i.gettype() == OBJECT_N)
		return "object";
	else{
		return "unknown";
	}
}
#define SIZE_OF(v) get_size(v);
#define IS_EMPTY(v) isEmpty(v);
#define HAS_KEY(v, key) has_key(v,#key);
#define TYPE_OF(v) typeof(v);

#define PRINT ;cout<<
//....................................
#endif // !JSON
