#ifndef VAR
#define VAR 1
#include "std_lib_facilities.h"

#define NOONTYPE_N -1
#define NULL_N 0
#define STR_N 1
#define NUM_N 2
#define BOOL_N 3
#define OBJECT_N 4
#define ARRAY_N 5

class item;
class Var;
class String;
class Array;
class Object;
void printval(item* v);

//tmp variables....
int tmp_append = 0;
//.....................
class Null{
public:
	const item* n = nullptr;
};

class Boolean{
	int bl;
	string bl_s;
protected:
	void set_bl(int b , string b_s){
		bl = b;
		bl_s = b_s;
	}
public:
	Boolean(){}
	Boolean(int b) :bl(b){
		if (b == 1)
			bl_s = "true";
		else if (b == 0)
			bl_s = "false";
	}
	string get_bl_s()const { return bl_s; }
	int get_bl()const { return bl; }
};
class Number{
protected:
	int ival;
public:
	Number(){}
	Number(int val) :ival(val){}
	int get_val(){ return ival; }
};
class String {
protected:
	string sval;
public:
	String(){ }
	String(string val) : sval(val) { }
	string get_val(){ return sval; }
};

class Array {
protected:
	vector<item> ar_val;
	void push(vector<item> &vi);
public:
	Array(){  }
	void push(item i);
	unsigned int size(){
		return ar_val.size();
	}
	item isEmpty();
	int search_ar(item *i); 
};
class Object {
protected:
	vector<item> obj_val;
public:
	Object(){  }
	item get_val(string key); 
	int search_obj(item *i);
	item* find(string key); 
	unsigned int size(){ return obj_val.size(); }
	item isEmpty(); 
};

class item :public String, public Array, public Object, public Number, public Boolean, public Null{
private:
	int type;
	string name;
protected:
	vector<item> tmp_vec;

public:
	item *parent = nullptr;
	item(string name) :Null(), type(NULL_N), name(name){}
	item() :Null(), type(NULL_N), name(""){}
	item(int type, string name, int bl) :Boolean(bl){
		if (type == OBJECT_N || type == ARRAY_N || type == BOOL_N){
			this->type = type;
			this->name = name;
		}
		else{
			this->type = 0;
			cout << "cannot recognize the type\n";
		}
	}

	item(string name, string strval) :String(strval), type(STR_N), name(name){  }
	item(string name, int num) : name(name), type(NUM_N), Number(num){}

	//friend void _erase(item* it, item* i, item* parent);
	friend void printval(item i);
	friend item& operator--(item &i);
	friend ostream& operator<<(ostream& cout, item& i);
	void set_name(string name){ this->name = name; }
	int gettype(){ return type; }
	string getname(){ return name; }
	item& operator=(const item &i);
	item& operator[](item &v){
		if (type == ARRAY_N){
			v.parent = this;
			this->Array::push(v);
			for (item it : v.tmp_vec){
				it.parent = this;
				this->Array::push(it);
			}
			v.tmp_vec.clear();
			this->ar_val[0].tmp_vec.clear();
		}
		return *this;
	}
	/*item& operator[](vector<item> vi){
		if (type == ARRAY_N){
		this->Array::push(vi);
		}
		return *this;
		}*/
	friend item& operator++(item &i);
	item& operator[](int i){
		if (i >= this->ar_val.size() || this->type != ARRAY_N){
			cout << "index out of bounds or item is not array, operation ignored\n";
			return *new item();
		}
		return this->ar_val[i];
	}
	item& operator[](string str){
		if (this->type != OBJECT_N){
			cout << "item is not object, operation ignored\n";
			return *new item();
		}
		item *ptr = nullptr;
		int j;
		for (j = 0; j < this->obj_val.size(); ++j){
			if (this->obj_val[j].name == str){
				ptr = &this->obj_val[j];
			}
		}
		if (ptr == nullptr){
			this->obj_val.push_back(*new item(str));
			obj_val.at(obj_val.size() - 1).parent = this;
			return  this->obj_val[this->obj_val.size() - 1];
		}
		return *ptr;
	}
	item& operator+=(vector<item> vi){
		if (type == OBJECT_N){
			this->obj_val = vi;
			int i;
			for (i = 0; i < this->obj_val.size(); ++i){
				this->obj_val[i].parent = this;
			}
		}
		return *this;
	}

	item& operator-=(item &i){
		tmp_append = 1;
		if (this->type != ARRAY_N){ cout << "append fail, is not array\n"; return *new item(); }
		i.parent = this;
		this->Array::push(i);
		return *this;
	}

	/*vector<item> operator,(item &i){
		vector<item> vi = { *this, i };
		return vi;
		}*/
	item& operator,(item &i){
		if (tmp_append)
			this->ar_val.push_back(i);
		else
			this->tmp_vec.push_back(i);
		return *this;
	}
	//........................................................
	item& operator+(item& i){
		if (this->type != i.gettype() || (i.gettype() != STR_N &&i.gettype() != NUM_N && i.gettype() != OBJECT_N && i.gettype() != ARRAY_N)){
			cout << "wrong use of operator+\n";
			item  *n = new item();
			return *n;
		}
		if (this->type == NUM_N){
			item *num = new item("", this->Number::get_val() + i.Number::get_val());
			return *num;
		}
		else if (this->type == STR_N){
			string s;
			s.append(this->String::get_val());
			s.append(i.String::get_val());
			item *str = new item("", s);
			return *str;
		}
		else if (this->type == ARRAY_N){
			for (item it : i.ar_val){
				this->push(it);
			}
			return *this;
		}
		else{
			for (item it : i.obj_val){
				this->obj_val.push_back(it);
			}
			return *this;
		}
	}
	item& operator-(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator-\n";
			item  *n = new item();
			return *n;
		}
		item *num = new item("", this->Number::get_val() - i.Number::get_val());
		return *num;

	}
	item& operator*(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator*\n";
			item  *n = new item();
			return *n;
		}
		item *num = new item("", this->Number::get_val() * i.Number::get_val());
		return *num;

	}
	item& operator/(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator/\n";
			item  *n = new item();
			return *n;
		}
		item *num = new item("", this->Number::get_val() / i.Number::get_val());
		return *num;

	}
	item& operator%(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator%\n";
			item  *n = new item();
			return *n;
		}
		item *num = new item("", this->Number::get_val() % i.Number::get_val());
		return *num;

	}
	item& operator>(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator>\n";
			item  *n = new item();
			return *n;
		}
		item *bl =new item( BOOL_N, "" ,this->ival>i.ival ) ;
		return *bl;
	}
	item& operator<(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator<\n";
			item  *n = new item();
			return *n;
		}
		item *bl = new item(BOOL_N, "", this->ival<i.ival);
		return *bl;
	}
	item& operator>=(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator>+\n";
			item  *n = new item();
			return *n;
		}
		item *bl = new item(BOOL_N, "", this->ival>=i.ival);
		return *bl;
	}
	item& operator<=(item& i){
		if (this->type != i.gettype() || i.gettype() != NUM_N){
			cout << "wrong use of operator<=\n";
			item  *n = new item();
			return *n;
		}
		item *bl = new item(BOOL_N, "", this->ival <= i.ival);
		return *bl;
	}
	item& operator&&(item& i){
		if (this->gettype() != BOOL_N && i.gettype() != BOOL_N){
			cout << "wrong use of operator&&\n";
			item  *n = new item();
			return *n;
		}
		if (this->get_bl() == 1 && i.get_bl() == 1){
			return *new item( BOOL_N , "" , 1);
		}
		return *new item(BOOL_N, "", 0);
	}
	item& operator||(item& i){
		if (this->gettype() != BOOL_N && i.gettype() != BOOL_N){
			cout << "wrong use of operator||\n";
			return *new item();
		}
		if (this->get_bl() == 1 || i.get_bl() == 1){
			return *new item(BOOL_N, "", 1);
		}
		return *new item(BOOL_N, "", 0);
	}
	item& operator==( item& i){
		if (i.gettype()!=type){
			*new item(BOOL_N, "", 0);
		}
		if (type == STR_N){
			if (this->String::get_val().compare(i.String::get_val()) == 0){
				return *new item(BOOL_N , "" , 1);
			}
		}
		else if (type == NUM_N){
			if (this->Number::get_val()==i.Number::get_val()){
				return *new item(BOOL_N, "", 1);
			}
		}
		else if (type == BOOL_N){
			if (this->get_bl()==i.get_bl()){
				return *new item(BOOL_N, "", 1);
			}
		}
		else if (type==NULL_N){
			return *new item(BOOL_N, "", 1);
		}
		else if (type == OBJECT_N){
			int j = 0;
			if (this->obj_val.size() != i.obj_val.size()){
				return *new item(BOOL_N, "", 0);
			}
			for (item it : this->obj_val){	
				item *ptr = i.find(it.getname());
				if (ptr == nullptr){
					return *new item(BOOL_N, "", 0);
				}
				if ( (*ptr== it).get_bl()){
				}
				else{
					return *new item(BOOL_N, "", 0);
				}
			}
			return *new item(BOOL_N, "", 1);
		}
		else if (type == ARRAY_N){
			int j = 0;
			if (this->ar_val.size() != i.ar_val.size())
				return *new item(BOOL_N, "", 0);
			for (item it : this->ar_val){
				if ((it == i.ar_val[j]).get_bl()!=1){
					return *new item(BOOL_N, "", 0);
				}
				++j;
			}
			return *new item(BOOL_N, "", 1);
		}
		else{
			cout << "unknown type:" << type << "\n";
		}

		return *new item(BOOL_N, "", 0);
	}
	item& operator!=(item& i){
		if (i.gettype() != type){
			*new item(BOOL_N, "", 0);
		}
		if (type == STR_N){
			if (this->String::get_val().compare(i.String::get_val()) == 0){
				return *new item(BOOL_N, "", 0);
			}
		}
		else if (type == NUM_N){
			if (this->Number::get_val() == i.Number::get_val()){
				return *new item(BOOL_N, "", 0);
			}
		}
		else if (type == BOOL_N){
			if (this->get_bl() == i.get_bl()){
				return *new item(BOOL_N, "", 0);
			}
		}
		else if (type == NULL_N){
			return *new item(BOOL_N, "", 0);
		}
		else if (type == OBJECT_N){
			int j = 0;
			if (this->obj_val.size() != i.obj_val.size()){
				return *new item(BOOL_N, "", 1);
			}
			for (item it : this->obj_val){
				item *ptr = i.find(it.getname());
				if (ptr == nullptr){
					return *new item(BOOL_N, "", 1);
				}
				if ((*ptr == it).get_bl()){
				}
				else{
					return *new item(BOOL_N, "", 1);
				}
			}
			return *new item(BOOL_N, "", 0);
		}
		else if (type == ARRAY_N){
			int j = 0;
			if (this->ar_val.size() != i.ar_val.size())
				return *new item(BOOL_N, "", 1);
			for (item it : this->ar_val){
				if ((it == i.ar_val[j]).get_bl() != 1){
					return *new item(BOOL_N, "", 1);
				}
				++j;
			}
			return *new item(BOOL_N, "", 0);
		}
		else{
			cout << "unknown type:" << type << "\n";
		}
		return *new item(BOOL_N, "", 1);
	}
	//................................................
};
item& operator!(item& i){
	if ( i.gettype() != BOOL_N){
		cout << "wrong use of operator&&\n";
	}
	if ( i.get_bl() == 0){
		return *new item(BOOL_N, "", 1);
	}
	return *new item(BOOL_N, "", 0);
}
item& operator--(item &i){
	if (i.gettype()==OBJECT_N || i.gettype()==ARRAY_N ){
		i.obj_val.clear();
		i.ar_val.clear();
	}
	else{
		if (i.parent == nullptr){
			cout  << "ERASE not valid input1\n";
		}
		if( i.parent->gettype() != ARRAY_N && i.parent->gettype() != OBJECT_N ){
		cout  << "ERASE not valid input2\n";
		}
		if (i.parent->gettype() == OBJECT_N){
			int pos = i.parent->search_obj(&i);
			i.parent->obj_val.erase(i.parent->obj_val.begin() + pos);
		}
		else{
			int pos = i.parent->search_ar(&i);
			i.parent->ar_val.erase(i.parent->ar_val.begin() + pos);
		}
	}
	return i;

}

item Object::get_val(string key){
	for (item it : obj_val){
		if (it.getname() == key)
			return it;
	}
}
void Array::push(item i){ ar_val.push_back(i); }
void Array::push(vector<item> &vi) { ar_val = vi; }
int Array::search_ar(item *i) {
	int z;
	for (z = 0; z < ar_val.size(); ++z){
		if (&ar_val[z] == i)
			return z;
	}
}
int Object::search_obj(item *i){
	int z ;
	for (z = 0; z < obj_val.size(); ++z){
		if (&obj_val[z] == i)
			return z;
	}
}
item* Object::find(string key){
	for (item it : this->obj_val){
		if (string(it.getname()).compare(key) == 0){
			return new item(it);
		}
	}
	return nullptr;
}

item Array::isEmpty(){
	if (ar_val.size() == 0)
		return *new item(BOOL_N, "", 1);
	return *new item(BOOL_N, "", 0);
}
item Object::isEmpty(){
	if (obj_val.size() == 0)
		return *new item(BOOL_N, "", 1);
	return *new item(BOOL_N, "", 0);
}
void printval(item i){
static int inAr = 0;
if (i.gettype() == STR_N){
inAr ? cout << "\"" << i.String::get_val() << "\" " : cout << i.getname() << "=\"" << i.String::get_val() << "\" ";
}
else if (i.gettype() == ARRAY_N){
inAr++;
(inAr <= 1) ? cout << i.getname() << "=[ " : cout << "[ ";
for (item it : i.ar_val){ printval(it); cout << " "; }
cout << "]";
inAr--;
}
else if (i.gettype() == OBJECT_N){
(inAr == 0) ? cout << i.getname() << "={ " : cout << "{ ";
for (item it : i.obj_val){ printval(it); }
cout << " } ";
}
else if (i.gettype() == NUM_N){
inAr ? cout << i.Number::get_val() << " " : cout << i.getname() << "=" << i.Number::get_val() << " ";

}
else if (i.gettype() == BOOL_N){
inAr ? cout << i.Boolean::get_bl_s() << " " : cout << i.getname() << "=" << i.Boolean::get_bl_s() << " ";

}
else if (i.gettype() == NULL_N){
inAr ? cout << "NULL" << " " : cout << i.getname() << "=" << "NULL" << " ";

}
else{
cout << " UNKNOWN_TYPE ";
}
}

#define IN_AR 1
#define IN_OBJ 0
vector<int> arobj;
ostream& operator<<(ostream& cout, item& i )
{
	if (i.gettype() == STR_N){
		if (arobj.size()==0){
			cout << "=" << "\"" << i.String::get_val() << "\" \n";
		}
		else{
			arobj.at(arobj.size()-1) == IN_AR ? cout << "\"" << i.String::get_val() << "\" " : cout << i.getname() << "=\"" << i.String::get_val() << "\" ";
		}
	}
	else if (i.gettype() == ARRAY_N){
		if (arobj.size()==0){
			cout << "=" << "[" ;
		}
		else{
			arobj.at(arobj.size() - 1) == IN_AR ? cout << "[" : cout << i.getname() << "=[";
		}
		arobj.push_back(IN_AR);
		for (item it : i.ar_val){ cout<<it; cout << " "; }
		cout << "] ";
		arobj.pop_back();
		if (arobj.size() == 0)
			cout << "\n";
	}
	else if (i.gettype() == OBJECT_N){
		if (arobj.size() == 0){
			cout << "=" << "{";
		}
		else{
			arobj.at(arobj.size() - 1) == IN_AR ? cout << "{" : cout << i.getname() << "={ ";
		}
		arobj.push_back(IN_OBJ);
		for (item it : i.obj_val){ cout << it; }
		cout << " } ";
		arobj.pop_back();
		if (arobj.size() == 0)
			cout << "\n";
	}
	else if (i.gettype() == NUM_N){
		if (arobj.size() == 0){
			cout << "=" << i.Number::get_val() << "\n";
		}
		else{
			arobj.at(arobj.size() - 1) == IN_AR ? cout << i.Number::get_val() << " " : cout << i.getname() << "=" << i.String::get_val() << " ";
		}
	}
	else if (i.gettype() == BOOL_N){
		if (arobj.size() == 0){
			cout << "=" << i.get_bl_s() << "\n";
		}
		else{
			arobj.at(arobj.size() - 1) == IN_AR ? cout << i.get_bl_s() << " " : cout << i.getname() << "=" << i.get_bl_s() << " ";
		}

	}
	else if (i.gettype() == NULL_N){
		if (arobj.size() == 0){
			cout << "=" << "NULL" << "\n";
		}
		else{
			arobj.at(arobj.size() - 1) == IN_AR ? cout << "NULL" << " " : cout << i.getname() << "=" << "NULL" << " ";
		}
	}
	else{
		cout << " UNKNOWN_TYPE \n";
	}
	return cout;
}

//#define JSON(p) ; tmp_name=#p ; tmp_js
item& item::operator=(const item &i){
        //this->name = i.name;
        this->type = i.type;
        this->parent = i.parent;
        if (i.type == STR_N){
                sval = i.sval;
        }
        else if (i.type==NUM_N){
                ival = i.ival;
        }
        else if (i.type==BOOL_N){
		int b=i.get_bl();
		string str=i.get_bl_s();
                set_bl(b , str );
        }
        else if (i.type==OBJECT_N){
                obj_val = i.obj_val;
                unsigned int k;
                for (k = 0; k < obj_val.size();++k){
                        obj_val[k].parent = this;
                }
        }else if (i.type == ARRAY_N){
                ar_val = i.ar_val;
                unsigned int k;
                for (k = 0; k < ar_val.size(); ++k){
                        ar_val[k].parent = this;
                }
        }
        else if (i.type == NULL_N){
        }
        else{
                cout << "otem::oparetion=() invalid type\n";
                return *new item();
        }
        return *this;
}


#endif
