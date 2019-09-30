#include <string>
#include <vector> 
#include <queue>
#include <utility> //for pair
#include <bits/stdc++.h> 

#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"  // Added cause of class llvm::ConstantInt
#include "llvm/IR/InstrTypes.h"

using namespace llvm;
using namespace std;

std::string if_branch = "n";


// Define class that represent interval of confidence for temporary variable
// Assusme that intervals are union of compact intervals ([a, b], where a and b are integers or +/- inf).
class IntervalOfConf{
	private: 
		std::string name;	
		// dva vektora parova - granice i indikatori beskonacnosti
		// 0 nije inf, 1 jeste inf
		vector< pair< llvm::APInt, llvm::APInt > > endpoints;
		vector< pair<int, int> > indicators;
		
	public:
		IntervalOfConf();
		IntervalOfConf(std::string);
		IntervalOfConf(std::string, const llvm::APInt, const llvm::APInt);

		void set_name(std::string);
		void set_value(const llvm::APInt, int);
		void set_bottom(const llvm::APInt, int);
		void set_top(const llvm::APInt, int);
		void set_both(const llvm::APInt,const llvm::APInt, int);
		bool is_bottom(int);
		bool is_top(int);

		llvm::APInt get_top(int);
		llvm::APInt get_bottom(int);
		std::string get_name();
		int get_number_of_intervals();
		
		void compress_intervals();
		
		void print();
};
IntervalOfConf::IntervalOfConf(){
	name = "-";
	pair<llvm::APInt, llvm::APInt> base;
	endpoints.push_back(base); 
	indicators.push_back(make_pair(1,1));
}

IntervalOfConf::IntervalOfConf(std::string _name){
	name = _name;
	pair<llvm::APInt, llvm::APInt> base;
	endpoints.push_back(base);
	indicators.push_back(make_pair(1,1));
}

IntervalOfConf::IntervalOfConf(std::string _name, const llvm::APInt _bottom, const llvm::APInt _top){
	name = _name;
	endpoints.push_back(make_pair(_bottom, _top));
	indicators.push_back(make_pair(0,0));
}

void IntervalOfConf::set_name(std::string _name){name = _name; }

void IntervalOfConf::set_value(const llvm::APInt _value, int index){
	
	if(index+1 > endpoints.size()){
		
		// moze i samo push_back xD
		endpoints.resize(index+1);
		indicators.resize(index+1);
		pair<llvm::APInt, llvm::APInt> base;
		endpoints[index] = base;
		indicators[index] = make_pair(1,1);
	}
	
	endpoints[index] = make_pair(_value, _value);
	indicators[index] = make_pair(0, 0);
	
}

void IntervalOfConf::set_bottom(const llvm::APInt _bottom, int index){
	
	if(index+1 > endpoints.size()){
		//endpoints.resize(index+1);
		//indicators.resize(index+1);
		pair<llvm::APInt, llvm::APInt> base;
		endpoints.push_back(base);
		indicators.push_back(make_pair(1,1));
	}
	
	endpoints[index].first = _bottom;
	indicators[index].first = 0;
	
}

void IntervalOfConf::set_top(const llvm::APInt _top, int index){
		
	if(index+1 > endpoints.size()){
		pair<llvm::APInt, llvm::APInt> base;
		endpoints.push_back(base);
		indicators.push_back(make_pair(1,1));
	}
	
	endpoints[index].second = _top;
	indicators[index].second = 0;
}

void IntervalOfConf::set_both(const llvm::APInt _bottom, const llvm::APInt _top, int index){
	
	if(index+1 > endpoints.size()){
		endpoints.resize(index+1);
		indicators.resize(index+1);
	}
	
	endpoints[index] = make_pair(_bottom, _top);
	indicators[index] = make_pair(0, 0);
		
}

bool IntervalOfConf::is_bottom(int index){
	
	return indicators[index].first != 1;
}

bool IntervalOfConf::is_top(int index){
	
	return indicators[index].second != 1; 
	
}

std::string IntervalOfConf::get_name(){return name; }

int IntervalOfConf::get_number_of_intervals(){return endpoints.size();}

llvm::APInt IntervalOfConf::get_top(int index){
	
	return endpoints[index].second;
	
}
llvm::APInt IntervalOfConf::get_bottom(int index){
	
	return endpoints[index].first;
}

// IMPLEMENTACIJA SPAJANJA INTERVALA

bool compare (pair<llvm::APInt,llvm::APInt> x, pair<llvm::APInt,llvm::APInt> y){
	
	int x_int = x.first.getSExtValue();
	int y_int = y.first.getSExtValue();
	
	return (x_int < y_int);
}

void IntervalOfConf::compress_intervals() {
	
	stack<pair<llvm::APInt,llvm::APInt>>  s; 
	vector<pair<llvm::APInt,llvm::APInt>> vect; 
	vector<pair<llvm::APInt,llvm::APInt>> bottom_inf, top_inf; 
	
	// kopiramo u vect samo one intervale koji imaju obe granice konacne
	for(int i = 0; i < endpoints.size(); i++)
		
		if (indicators[i].first == 1 && indicators[i].second == 1){
			
			// postoji interval cije su granice -inf, +inf --> to je cela unija!
			endpoints.resize(1);
			indicators.resize(1);
			pair<llvm::APInt, llvm::APInt> base;
			endpoints[0] = base;
			indicators[0] = make_pair(1,1);
			return;
		}
		
		else if(indicators[i].first == 0 && indicators[i].second == 0){
			
			vect.push_back(endpoints[i]); // konacni intervali
			
		}
		else if (indicators[i].first == 1) {
			
			bottom_inf.push_back(endpoints[i]); // intervali cija je donja granica -inf
			
		}
		else {
			
			top_inf.push_back(endpoints[i]); // intervali cija je gornja granica +inf
		}
		
		
		// OBRADA BESKONACNOSTI
		// bottom_inf --> [-inf, 2] U [-inf, 10] U [-inf, 4] --> unija je ocigledno [-inf, 10], odnosno interval sa najvecom gornjom granicom
		// top_inf --> [25, inf] U [11, inf] U [30, inf] --> unija je ocigledno [11, inf], odnosno interval sa najmanjom donjom granicom
		// vidimo da se [-inf, 10] i [11, inf] mogu spojiti u jedan interval: [-inf, inf], tako da cemo imati proveru da li se dva rezultujuca intervala mogu preklopiti, sa odredjenom preciznoscu, odnosno uzmemo gornju granicu od bottom_inf i donju granicu od top_inf i uporedimo da li se razlikuju za najvise precision
		
		bool ind_bottom = false;
		pair<llvm::APInt, llvm::APInt> max_top;
		if (bottom_inf.size() > 0){
			ind_bottom = true;
			max_top = bottom_inf[0];
			for (int i=1; i < bottom_inf.size(); i++){
				
				if (bottom_inf[i].second.getSExtValue() > max_top.second.getSExtValue())
					
					max_top = bottom_inf[i];
			}	
		}
		
		bool ind_top = false;
		pair<llvm::APInt, llvm::APInt> min_bottom;
		if (top_inf.size() > 0){
			ind_top = true;
			min_bottom = top_inf[0];
			for (int i=1; i < top_inf.size(); i++){
				
				if (top_inf[i].first.getSExtValue() < min_bottom.first.getSExtValue())
					
					min_bottom = top_inf[i];
			}	
		}
		
		
		// provera da li mozemo da spojimo max_top i min_bottom
		
		if(ind_top && ind_bottom && max_top.second.getSExtValue() >= min_bottom.first.getSExtValue()) {
			
			// unija je [-inf, inf]
			
			endpoints.resize(1);
			indicators.resize(1);
			pair<llvm::APInt, llvm::APInt> base;
			endpoints[0] = base;
			indicators[0] = make_pair(1,1);
			return;
		}
		
		// inace nam ostaju max_top i min_bottom
		// OBRADA KONACNIH INTERVALA
		// npr da imamo sledecu situaciju
		// [3, 11] U [20, 30] U [1, 9] U [25, 41] = [1, 11] U [20, 41]
		// bas parovi (1,11) i (20, 41) ce biti rezultat izvrsavanja narednog dela programa, u tom poretku (smesteni u vektor compressed)
		
		int n = vect.size();
		
		vector<pair<llvm::APInt, llvm::APInt>> compressed;
		
		if(n!=0) {
			// sortiranje prema donjoj granici
			sort(vect.begin(), vect.end(), compare); 
			
			// dodajemo prvi interval na stek
			s.push(vect[0]); 
			
			// pocinjemo od sledeceg intervala i spajamo po potrebi 
			for (int i = 1 ; i < n; i++) 
			{ 
				// skini interval sa steka
				pair<llvm::APInt,llvm::APInt> top = s.top(); 
		 
				// ako se trenutni interal ne preklapa sa intervalom vrhom steka, 
				// stavi ga na stek
				if (top.second.getSExtValue() < vect[i].first.getSExtValue()) 
					s.push(vect[i]); 

				// u suprotnom, azurirati gornju granicu vrha ukoliko je gornja granica 
				// tekuceg intervala veca
				else if (top.second.getSExtValue() < vect[i].second.getSExtValue()) 
				{ 
					top.second = vect[i].second; 
					s.pop(); 
					s.push(top); 
				} 
			}
			
			while (!s.empty()) 
			{ 
				pair<llvm::APInt,llvm::APInt> t = s.top(); 
				compressed.push_back(t);
				s.pop(); 
			} 
			
			sort(compressed.begin(), compressed.end(), compare); 
		}
		
		// recimo da nakon ovoga imamo situaciju [-inf, 2] U [-3, 0] U [20, 41] U [65, 100] U [60, inf]
		// treba eventualno jos spojiti prva dva intervala i poslednja dva intervala u jedan
		
		n = compressed.size();
		
		if(n > 0 && ind_bottom){
			
			if(max_top.second.getSExtValue() >= compressed[0].second.getSExtValue()){ // prvi konacan interval upada u [-inf, 2]
				
				compressed[0] = max_top;
			}
			
			else if(max_top.second.getSExtValue() >= compressed[0].first.getSExtValue()){ // beskonacan interval se moze produziti do gornje granice konacnog
				
				compressed[0].first = max_top.first;
			}
			
			else { // inace dodajemo u compressed na pocetak beskonacan interval
				
				compressed.insert(compressed.begin(), max_top);
				
			}
			
		}
		
		n = compressed.size(); // mozda se povecalo za 1!
		
		if (n > 0 && ind_top){
			
			if(compressed[n-1].first.getSExtValue() >= min_bottom.first.getSExtValue()){ //poslednji konacan interval upada u [60, inf]
				
				compressed[n-1] = min_bottom; 
			}
			
			else if (compressed[n-1].second.getSExtValue() >= min_bottom.first.getSExtValue()) { // beskonacan interval se moze produziti do donje granice konacnog
				
				compressed[n-1].second = min_bottom.second;
			}
			
			else { // inace dodajemo u compressed na kraj beskonacan interval
				
				compressed.push_back(min_bottom);
			}
		}
		
		indicators.resize(compressed.size());
		
		for (int i=0; i < compressed.size(); i++){
			
			if (i == 0 && ind_bottom){
				
				indicators[i] = make_pair(1, 0); // donja granica prvog intervala je -inf
			}
			
			else if (i == compressed.size()-1 && ind_top) { // gornja granica poslednjeg intervala je inf
				
				indicators[i] = make_pair(0, 1);
				
			}
			
			else {
				
				indicators[i] = make_pair(0, 0); // sve izmedju su konacni intervali
			}
		}
		
		endpoints.resize(compressed.size());
		endpoints = compressed; // u endpoints samo kopiramo elemente iz compressed
}


void IntervalOfConf::print(){
	outs()<<name<<" in: ";
	
	for (int i = 0; i < endpoints.size() ; i++){
		
		outs()<<"[";
		if(indicators[i].first == 1)
			outs()<<"-inf, ";
		else
			outs()<<endpoints[i].first<<", ";
		
		if(indicators[i].second == 1)
			outs()<<"+inf]";
		else
			outs()<<endpoints[i].second<<"]";
		
		if(i+1 == endpoints.size()) // ako smo dosli do poslednjeg intervala
			outs()<<"\n";
		else
			outs()<<" U "; // inace dodajemo znak za uniju i idemo dalje
	}
  }	

// Our pass is subfunction of class FunctionPass
class AIProlaz : public FunctionPass
{
public:
	// Deklarismo identifikator prolaza koji LLVM koristi da bi identifikovao prolaz
	static char ID;
	AIProlaz() : FunctionPass(ID)
	{}

	// Override-ujemo metod koji je vec definisan u llvm-u, ovde je sustina programa
	bool runOnFunction(Function& F) override
	{
		// Inicijalizujemo listu intervala na osnovu prosledjene funkcije
		std::queue<Value*> lista_intervala = init(F);
		outs() << "Function name: "<< F.getName()<<"\n"; 
		outs() << "Number of parsed function instruction: " << lista_intervala.size() << "\n";
		outs() << "********************************************************************************\n";  // cause of fine output
		outs() << "Intervals of confidence: \n";
		for(auto it = _intervals.cbegin(); it != _intervals.cend(); ++it)
		{	
    	outs()<< *(it->first) << " \n\t"; 
			if(it->second!=nullptr){
				IntervalOfConf* up_to_date = findByName(it->second->get_name());
				if(up_to_date!=nullptr)
					up_to_date->print();
			}else{
				outs()<<"nullptr\n";
			}
		}
		outs() << "********************************************************************************\n";
		
		// Adding to function list; Cause of function composition
		IntervalOfConf* i = findByName("RETURN_VALUE");
		if(i!=nullptr)
			i->print();
		_functions[F.getName()] = i;
		outs() << "Current parsed functions: \n";  // Printout functons parsed till now.
		for(auto it = _functions.cbegin(); it != _functions.cend(); ++it){
			outs()<< it->first<< " \n\t"; 
			if(it->second!=nullptr)
				it->second->print();
			else
				outs()<<"nullptr\n";
		}

		// Return false cause of program isnt changed. LLVM Standard.
		return false;
   }

private:
	std::list<std::pair<Value*, IntervalOfConf*>> _intervals;  // List of parsed intervals. Value* represent instruction
	std::map<std::string, IntervalOfConf*> _functions;         // List of parsed functions. std::string represent function name

	IntervalOfConf* _parseInterval(Value* vred){
		// Parsing function arguments dependently
		if (auto *funk_arg = dyn_cast<Argument>(vred))
		{
			outs() << "Obradjena vrednost je [argument funkcije]" << "\n";
			outs() << "Ime funkcije: " << funk_arg->getParent() << "\n";
			
			return nullptr;
		}

		int opcode = (dyn_cast<Instruction>(vred))->getOpcode();  // Parse opcode of instruction
		
		switch(opcode){  // Util function for each supported instruction
			case 29:
				return parseAlloca(vred);
				break;
			case 30:
				return parseLoad(vred);
				break;
			case 31:
				return parseStore(vred);
				break;
			case 11:
				return parseAdd(vred);
				break;
			case 13:
				return parseSub(vred);
				break;
			case Instruction::Mul: // dodata operacija mnozenja
				return parseMul(vred);
				break;
			case Instruction::SDiv: // dodata operacija deljenja
				return parseDiv(vred);
				break;
			case 51:
				return parseIcmp(vred);
				break;
			case 54:
				return parseCall(vred);
				break;
			case 2:
				return parseBr(vred);
				break;
			case 1:
				return parseRet(vred);
			default:
				outs() << "-- not supported --";
		}
	outs() << "\n";

	return nullptr;
	}

	// Utils:

	IntervalOfConf* parseLoad(Value* vred){
		outs() << "WORKING ON: load...\n";
		Instruction* inst = cast<Instruction>(vred);
		assert(inst);		

		/* 
			example: "%tmp2 = load i32, i32* %tmp1, align 4"  (*inst)
			load to: "tmp2"                                   (inst->getName())
			from:    "tmp1"                                   ((inst->getOperand(0))->getName())
		*/
		IntervalOfConf* i = new IntervalOfConf(inst->getName()); 		
		
		std::string name = (inst->getOperand(0))->getName(); 
		IntervalOfConf* j = findByName(name);
		
		if ( j!= nullptr ){
		
			for(int ind = 0; ind < j->get_number_of_intervals(); ind++ ){
			
			if (j->is_bottom(ind))
				i->set_bottom(j->get_bottom(ind), ind);
			
			if(j->is_top(ind))
				i->set_top(j->get_top(ind),ind);
			
				}
		}
		
		else {
			// Nothing, i is in [-inf, +inf] anyway
		}

		outs() <<"DONE: load.\n";
		return i;
	}

	IntervalOfConf* parseStore(Value* vred){
		outs() << "WORKING ON: store...\n";
		Instruction* inst = cast<Instruction>(vred);  // store i32 7, i32* %tmp1, align 4
		assert(inst);	
		
		Value* v1 = inst->getOperand(0);  // v1: "0xaa9400", *v1: "i32 7", v1->getName(): ""
		Value* v2 = inst->getOperand(1);  // v2: "0xaa6848", *v2: "%tmp1 = alloca i32, align 4", v2->getName(): tmp1
 		
		IntervalOfConf* i;
		int indexing = 0;
		BasicBlock* BB = inst->getParent();
		if (BB->getName() == if_branch){
			i = findByName(v2->getName());
			indexing = i->get_number_of_intervals();
		}
		else {
			i = new IntervalOfConf();
			i->set_name(v2->getName());
			
		}


		if(v1->getName()==""){
			// store from const int
		  Instruction *_inst2 = dyn_cast<Instruction>(v2);
		
			if (auto* konst_int = dyn_cast<ConstantInt>(v1))
			{
				i->set_value(konst_int->getValue(), indexing); //ako je store from const int
			}else{
				outs() << "STORE ERROR!\n";
			}
		}else{
			// store from virtual register
			std::string name = v1->getName(); 
			IntervalOfConf* j = findByName(name);

			if(j!=nullptr){
				
				for (int ind = 0; ind < j->get_number_of_intervals(); ind++ ){
					
					if (j->is_bottom(ind))
						i->set_bottom(j->get_bottom(ind),ind + indexing);
					
					if (j->is_top(ind))
						i->set_top(j->get_top(ind),ind + indexing);
					
				}
				
			} else{
				// Nothing, i is in [-inf, +inf] anyway
			}
		}

		outs() <<"DONE: store.\n";
		return i;
	}

	IntervalOfConf* parseAlloca(Value* vred){
		outs() << "WORKING ON: alloca...\n";
		Instruction* inst = cast<Instruction>(vred);  // %tmp = alloca i32, align 4
		assert(inst);	
		IntervalOfConf* i = new IntervalOfConf();

		i->set_name(inst->getName());  // Just set name, no any values

		outs() << "DONE: alloca.\n";
		return i;
	}

	IntervalOfConf* parseRet(Value* vred){
		outs() << "WORKING ON: ret...\n";
		Instruction* inst = cast<Instruction>(vred);  // ret i32 %tmp7 0x11879c0
		assert(inst);		

		IntervalOfConf* i = new IntervalOfConf("RETURN_VALUE");	 // Hardcoded name		
		
		std::string name = (inst->getOperand(0))->getName(); 
		IntervalOfConf* j = findByName(name);
		if(j!=nullptr){
			
			for (int ind = 0; ind < j->get_number_of_intervals(); ind++ ){
				
				if (j->is_bottom(ind))
					i->set_bottom(j->get_bottom(ind),ind);
				
				if (j->is_top(ind))
					i->set_top(j->get_top(ind),ind);
			}
			
		}else{
			// Nothing, i is in [-inf, +inf] anywhay
		}
		
		outs() << "DONE: ret.\n";
		return i;
	}

	IntervalOfConf* parseAdd(Value* vred){
		outs() << "WORKING ON: add...\n";
		Instruction* inst = cast<Instruction>(vred);  // %tmp6 = add nsw i32 %tmp4, %tmp5 0x0
		assert(inst);		

		Value* v1 = inst->getOperand(0);  // v1: "0x21e6b68", *v1: "%tmp4 = load i32, i32* %tmp1, align 4", v1->getName(): "tmp4"
		Value* v2 = inst->getOperand(1);  // v2: "0x21e73d8", *v2: "%tmp5 = load i32, i32* %tmp2, align 4", v2->getName(): "tmp5"
		assert(v2);
		
		IntervalOfConf* i = new IntervalOfConf(inst->getName()); // rezultujuci interval
		IntervalOfConf* j = findByName(v1->getName()); // interval prvog operanda
		IntervalOfConf* k = findByName(v2->getName()); // interval drugog operanda
		
		if(j!=nullptr && k!=nullptr){  // [3, 6]U[2,8] + [4, 7] = [7, 13]U[6,15] --> kompresija: [6, 15]
			
			int ind_i = 0;
			
			for(int ind_j = 0; ind_j < j->get_number_of_intervals(); ind_j ++ ){
				
				
				for(int ind_k = 0; ind_k < k->get_number_of_intervals(); ind_k ++ ){
						
							if(j->is_bottom(ind_j) && k->is_bottom(ind_k))
								i->set_bottom(j->get_bottom(ind_j) + k->get_bottom(ind_k), ind_i);
							
							if(j->is_top(ind_j) && k->is_top(ind_k))
								i->set_top(j->get_top(ind_j) + k->get_top(ind_k), ind_i);
							
							ind_i++;
						
				}
						
		
			}	
			
			i->compress_intervals();	
		}else{
			// nothing. Result is unknown if I don't know any of j or k (i=j+k)
		}

		outs() << "DONE: add.\n";
		return i;
	}

	IntervalOfConf* parseSub(Value* vred){
		outs() << "WORKING ON: sub...\n";
		Instruction* inst = cast<Instruction>(vred);  
		assert(inst);		

		Value* v1 = inst->getOperand(0);  
		Value* v2 = inst->getOperand(1);  
		assert(v2);
		
		IntervalOfConf* i = new IntervalOfConf(inst->getName());	
		IntervalOfConf* j = findByName(v1->getName());
		IntervalOfConf* k = findByName(v2->getName());
		
		if(j!=nullptr && k!=nullptr){  // j-k: [3, 6]U[1,2] - [4, 7] = [-7, 2]U[-6,-2] -->kompresija [-7,2]
			
			int ind_i = 0;
			
			for(int ind_j = 0; ind_j < j->get_number_of_intervals(); ind_j ++ ){
				
				
				for(int ind_k = 0; ind_k < k->get_number_of_intervals(); ind_k ++ ){
					
					if(j->is_bottom(ind_j) && k->is_top(ind_k))
						i->set_bottom(j->get_bottom(ind_j) - k->get_top(ind_k), ind_i);
					
					if(j->is_top(ind_j) && k->is_top(ind_k))
						i->set_top(j->get_top(ind_j) - k->get_bottom(ind_k), ind_i);
					
					ind_i++;
					
				}
				
					
			}	
			
			i->compress_intervals();

		}else{
			// nothing. Result is unknown if I don't know any of j or k (i=j+k)
		}

		outs() << "DONE: sub.\n";
		return i;
	}

	// Instrukcija mul vraca proizvod svojih operanada
	// <result> = mul i32 4, %var --> daje i32:result = 4 * %var	
	
	IntervalOfConf* parseMul(Value* vred){
		outs() << "WORKING ON: mul...\n";
		Instruction* inst = cast<Instruction>(vred);  
		assert(inst);		
		
		Value* v1 = inst->getOperand(0);  
		Value* v2 = inst->getOperand(1);  
		assert(v2);
		
		IntervalOfConf* i = new IntervalOfConf(inst->getName());	
		IntervalOfConf* j = findByName(v1->getName());
		IntervalOfConf* k = findByName(v2->getName());
		if(j!=nullptr && k!=nullptr){  // j * k: [2, 4]U[1,5] * [3, 10] = [6, 40]U[3,50]
			
			int ind_i = 0;
			
			for(int ind_j = 0; ind_j < j->get_number_of_intervals(); ind_j ++ ){
				
				
				for(int ind_k = 0; ind_k < k->get_number_of_intervals(); ind_k ++ ){
					
					if(j->is_bottom(ind_j) && k->is_bottom(ind_k))
						i->set_bottom(j->get_bottom(ind_j) * k->get_bottom(ind_k), ind_i);
					
					if(j->is_top(ind_j) && k->is_top(ind_k))
						i->set_top(j->get_top(ind_j) * k->get_top(ind_k), ind_i);
					
					ind_i++;
					
				}
				
				
			}
			
			i->compress_intervals();
		
		}else{
			// nothing. Result is unknown if I don't know any of j or k (i=j+k)
		}
		
		outs() << "DONE: mul.\n";
		return i;
	}
	
	
	//Instrukcija sdiv vraca celobrojni kolicnik dva cela broja
	// <result> = sdiv i32 4, %var --> daje i32:result = 4 / %var
	// deljenje dva cela broja u C++ daje ceo broj (odnosno celobrojni kolicnik, ako nema ekpslicitnog kastovanja operanada)
	IntervalOfConf* parseDiv(Value* vred){
		outs() << "WORKING ON: sdiv...\n";
		Instruction* inst = cast<Instruction>(vred);  
		assert(inst);		
		
		Value* v1 = inst->getOperand(0);  
		Value* v2 = inst->getOperand(1);  
		assert(v2);
		
		IntervalOfConf* i = new IntervalOfConf(inst->getName());	
		IntervalOfConf* j = findByName(v1->getName());
		IntervalOfConf* k = findByName(v2->getName());
		if(j!=nullptr && k!=nullptr){  // j / k: [4, 10] / [5, 15] = [4 / 15, 10 / 5] = [0, 2]
			int ind_i = 0;
			for (int ind_j = 0; ind_j < j->get_number_of_intervals(); ind_j++){
			
				for (int ind_k = 0; ind_k < k->get_number_of_intervals(); ind_k++){
				
					if(j->is_bottom(ind_j) && k->is_top(ind_k)){
						
						int del1 = j->get_bottom(ind_j).getSExtValue();
						int del2 = k->get_top(ind_k).getSExtValue();
						int kol = del1/del2;
						i->set_bottom(llvm::APInt(32, kol ,true), ind_i);
						
					}
					if(j->is_top(ind_j) && k->is_bottom (ind_k)){
						int del1 = j->get_top(ind_j).getSExtValue();
						int del2 = k->get_bottom(ind_k).getSExtValue();
						int kol = del1/del2;
						i->set_top(llvm::APInt(32, kol ,true), ind_i);
						
					}
					
					ind_i ++ ;
				}
			}
			
			i->compress_intervals();
			
		}else{
			// nothing. Result is unknown if I don't know any of j or k (i=j+k)
		}
		
		outs() << "DONE: sdiv.\n";
		return i;
	}
	// LLVM Reference manual: https://llvm.org/docs/LangRef.html#icmp-instruction
	// <result> = icmp <cond> <ty> <op1>, <op2>
	// <cond>:        sgt(>), sge(>=), slt(<), sle(<=)
	// <op1>, <op2>:  The remaining two arguments must be integer or pointer or integer vector typed. They must also be identical types.
	// Currently covering:
	// pointer, integer (%tmp<3)
	// integer, pointer (7>=%tmp)
	IntervalOfConf* parseIcmp(Value* vred){
		outs() << "WORKING ON: icmp...\n";
		Instruction* inst = cast<Instruction>(vred);  
		assert(inst);		
		
		Value* v1 = inst->getOperand(0);  // %tmp2 = load i32, i32* %tmp1, align 4
		Value* v2 = inst->getOperand(1);  // i32 5
		assert(v2);
		IntervalOfConf *l1, *l2;
		if(auto* konst_int = dyn_cast<ConstantInt>(v2)){ // slucaj kad je drugi operand celobrojna konstanta
			if(cast<Instruction>(v1)->getOpcode()==30){ // prvi operand je instrukcija load, odnosno ucitavanje vrednosti iz virtualnog registra
				// %tmp < 5
				// *inst: "%tmp3 = icmp slt i32 %tmp2, 5"
				
				/* 
				 example: "%tmp2 = load i32, i32* %tmp1, align 4"  (*inst)
				 load to: "tmp2"                                   (inst->getName())
				 from:    "tmp1"                                   ((inst->getOperand(0))->getName())
				 */
				
				auto* konst_int = dyn_cast<ConstantInt>(v2);
				assert(konst_int); 
				llvm::APInt B = konst_int->getValue();  // Border
				
				l1 = findByName(v1->getName()); // registar u koji se ucitava
				l2 = findByName((cast<Instruction>(v1)->getOperand(0))->getName()); // registar iz kojeg se ucitava
				// l1 is loaded from l2; change both of them
				
				CmpInst *cp = dyn_cast<CmpInst>(vred);
				assert(cp);
				
				if(cp->getPredicate()==CmpInst::ICMP_SLT){  // newly splitted (date: 03.09.2019.)
					// ako je u pitanju instrukcija slt (<) onda menjamo gornje granice intervala l1 i l2 tako da bude B ukoliko je beskonacna ili > od B
					// tmp < B --> postavljamo interval od tmp na [whatever, B]
				
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++){
					
						if(!l1->is_top(ind))
							l1->set_top(B, ind);
						else {
							
							if(l1->get_top(ind).sge(B))
								l1->set_top(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++) {
					
						if(!l2->is_top(ind))
							l2->set_top(B, ind);
						else {
							
							if(l2->get_top(ind).sge(B))
								l2->set_top(B, ind);	
						}
					}
				}
				else if(cp->getPredicate()==CmpInst::ICMP_SLE){
					// ako je u pitanju instrukcija sle (<=) onda menjamo gornje granice intervala l1 i l2 tako da bude B+1 ukoliko je beskonacna ili > od B
					// tmp <= B --> postavljamo interval od tmp na [whatever, B+1]
					B = B + 1;
					
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++){
						
						if(!l1->is_top(ind))
							l1->set_top(B, ind);
						else{
							if(l1->get_top(ind).sge(B))
								l1->set_top(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++ ){
						
						if(!l2->is_top(ind))
							l2->set_top(B, ind);
						else{
							if(l2->get_top(ind).sge(B))
								l2->set_top(B, ind);	
						}
					}
					
				}else if(cp->getPredicate()==CmpInst::ICMP_SGT){  // newly added 
					// sgt, "tmp>7"
					
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++){
					
						if(!l1->is_bottom(ind))
							l1->set_bottom(B, ind);
						else{
							if(l1->get_bottom(ind).sle(B))
								l1->set_bottom(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++ ){
					
						if(!l2->is_bottom(ind))
							l2->set_bottom(B, ind);
						else{
							if(l2->get_bottom(ind).sle(B))
								l2->set_bottom(B, ind);	
						}
					}
				}else if(cp->getPredicate()==CmpInst::ICMP_SGE){
					// sge, "tmp>=7"
					B -= 1;
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++ ){
					
						if(!l1->is_bottom(ind))
							l1->set_bottom(B, ind);
						else{
							if(l1->get_bottom(ind).sle(B))
								l1->set_bottom(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++ ){
						
						if(!l2->is_bottom(ind))
							l2->set_bottom(B, ind);
						else{
							if(l2->get_bottom(ind).sle(B))
								l2->set_bottom(B, ind);	
						}
					}
				}	
			}
		}else if(auto* konst_int = dyn_cast<ConstantInt>(v1)){
			if(cast<Instruction>(v2)->getOpcode()==30){
				// 5 > %tmp
				// *inst: "%tmp3 = icmp sgt i32 5, %tmp2"
				auto* konst_int = dyn_cast<ConstantInt>(v1);
				assert(konst_int); 
				llvm::APInt B = konst_int->getValue();  // Border
				
				l1 = findByName(v2->getName());
				l2 = findByName((cast<Instruction>(v2)->getOperand(0))->getName());  // l1 is loaded from l2; change both of them
				
				CmpInst *cp = dyn_cast<CmpInst>(vred);
				assert(cp);
				if(cp->getPredicate()==CmpInst::ICMP_SLT){  // newly added  
					// slt, <; "5 < %tmp"  [5, +inf] 5 not included
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++ ){
					
						if(!l1->is_bottom(ind))
							l1->set_bottom(B, ind);
						else{
							if(l1->get_bottom(ind).sle(B))
								l1->set_bottom(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++ ){
					
						if(!l2->is_bottom(ind))
							l2->set_bottom(B, ind);
						else{
							if(l2->get_bottom(ind).sle(B))
								l2->set_bottom(B, ind);	
						}
					}
					
				}else if(cp->getPredicate()==CmpInst::ICMP_SLE){
					// sle, <=; "5 <= %tmp"  [4, +inf] 4 not included
					B -= 1;
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++ ){
					
						if(!l1->is_bottom(ind))
							l1->set_bottom(B, ind);
						else{
							if(l1->get_bottom(ind).sle(B))
								l1->set_bottom(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++ ){
					
						if(!l2->is_bottom(ind))
							l2->set_bottom(B, ind);
						else{
							if(l2->get_bottom(ind).sle(B))
								l2->set_bottom(B, ind);	
						}
					}
					
				}else if(cp->getPredicate()==CmpInst::ICMP_SGT){  // newly added 
					// sgt, >; "5 > %tmp"  [-inf, 5], 5 not included
					
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++ ){
					
						if(!l1->is_top(ind))
							l1->set_top(B, ind);
						else{
							if(l1->get_top(ind).sge(B))
								l1->set_top(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++ ){
					
						if(!l2->is_top(ind))
							l2->set_top(B, ind);
						else{
							if(l2->get_top(ind).sge(B))
								l2->set_top(B, ind);	
						}
					}
					
				}else if(cp->getPredicate()==CmpInst::ICMP_SGE){
					// sge, >=; "5 >= %tmp"  [-inf, 6], 6 not included
					B += 1;
					
					for (int ind = 0; ind < l1->get_number_of_intervals(); ind++ ){
					
						if(!l1->is_top(ind))
							l1->set_top(B, ind);
						else{
							if(l1->get_top(ind).sge(B))
								l1->set_top(B, ind);
						}
					}
					
					for (int ind = 0; ind < l2->get_number_of_intervals(); ind++ ){
					
						if(!l2->is_top(ind))
							l2->set_top(B, ind);
						else{
							if(l2->get_top(ind).sge(B))
								l2->set_top(B, ind);	
						}
					}
				}
			}
		}else{
			outs() << "ICMP operation currently not supported.\n";  // Not parse assertion; Not updating any of variables
			return nullptr;
		}
		
		// tmp3 is true (1) or false (0)!
		// l1 and l2 should be the same at this place; Working with l1.
		IntervalOfConf* i = new IntervalOfConf(inst->getName());
		
		for (int ind = 0; ind < l1->get_number_of_intervals(); ind++ ){
		
			if(l1->is_bottom(ind) && l1->is_top(ind) && l1->get_bottom(ind).sgt(l1->get_top(ind))){
			
				continue; // interval sa indeksom ind ne odgovara, idemo na sledeci
			
				//}else if(l1->is_bottom() && l1->is_top() && l1->get_bottom().sge(l1->get_top()) &&  l1->get_bottom().sle(l1->get_top())){ TODO: Add check when interval is only one number
				//	outs()<<"Assertion is satisfied, for shure.\n";
				//	i->set_both(llvm::APInt(32, 1, true), llvm::APInt(32, 1, true));  // result is true, aka. [1, 1]
			} else {
				outs()<<"Assertion could be satisfied.\n";
				i->set_both(llvm::APInt(32, 0, true), llvm::APInt(32, 1, true), 0);  // result is possible true, aka. [0, 1]
				outs() << "DONE: icmp.\n";
				return i;  // I %tmp3: [0, 0] or [0, 1] (false or possibly true)
				// bar jedan interval iz unije zadovoljava assert
			}
		}
		
		// ako smo izvrteli celu petlju  - nije nadjen nijedan interval koji potencijalno zadovoljava assert
		outs()<<"Assertion failed in interval analysis time.\n";
		i->set_both(llvm::APInt(32, 0, true), llvm::APInt(32, 0, true), 0);  // result is false, aka. [0, 0]
		outs() << "DONE: icmp.\n";
		return i;  // I %tmp3: [0, 0] or [0, 1] (false or possibly true)
	}


	// Only direct call function of one argument! 
	IntervalOfConf* parseCall(Value* vred){
		outs() << "WORKING ON: call...\n";
		Instruction* inst = cast<Instruction>(vred);  // "%tmp7 = call i32 @_Z1gi(i32 %tmp6)"
		assert(inst);		

		outs() << "Parse call: \n";
		outs() << "inst->getName(): "<<inst->getName()<<"\n";  // %tmp7
		outs() << "inst->getOperand(0)->getName(): "<<inst->getOperand(0)->getName()<<"\n";  // tmp6

		CallInst* cinst = cast<CallInst>(vred);
		outs()<<"Real name: "<<cinst->getName().str()<<"\n";
		Function *fun = cinst->getCalledFunction();
		if (fun) 
    	outs()<<"Function name: "<<fun->getName().str()<<"\n"; //_Z1gi
		else
			outs()<<"Unnamed function\n";

		outs() << "Argument: " << inst->getOperand(0)->getName() << "\n";
		outs() << "Interval of confidence: ";
		IntervalOfConf* i = findByName(inst->getOperand(0)->getName());
		if(i!=nullptr)		
			i->print();
		else
			outs() << "nullptr\n";
		outs() << "Function: " << fun->getName().str() << "\n";
		outs() << "Interval of confidence: ";
		IntervalOfConf* j = _functions[fun->getName().str()];
		if(j!=nullptr)		
			j->print();
		else
			outs() << "nullptr\n";

		IntervalOfConf* r = new IntervalOfConf(inst->getName());  // ok, return only j; I can combine in future.
		
		if(j!=nullptr){
		
			for(int ind = 0; ind < j->get_number_of_intervals(); ind++){
				if(j->is_bottom(ind))
					r->set_bottom(j->get_bottom(ind), ind);
				if(j->is_top(ind))
					r->set_top(j->get_top(ind), ind);
			}
		}

		outs() << "DONE: call.\n";
		return r;
	}

	IntervalOfConf* parseBr(Value* vred){
		outs() << "WORKING ON: br...\n";
		Instruction* inst = cast<Instruction>(vred);  // "br i1 %tmp3, label %bb4, label %bb5"
		assert(inst);		
		
		BranchInst* binst = cast<BranchInst>(vred);
		
		unsigned b = binst->getNumSuccessors(); 
		
		// succesors - blokovi na koje naredba grananja moze da se razgrana
		
		if(b==1){
			// br label %bb6 - naredba bezuslovnog skoka
			// Ok nothing to do, it will come anywhay
		}else if(b==2){
			// "br i1 %tmp3, label %bb4, label %bb5"
			// ovde imamo razlicite blokove u zavisnosti od ispunjenosti uslova
			
			IntervalOfConf* i = findByName(inst->getOperand(0)->getName());  // it should be [0, 0] (false first branch) or [0, 1] (possible both)
			// i nam je interval koji vraca funkcija parseIcmp
			// ako je interval i [0, 0] onda znamo kojom granom program nastavlja izvrsavanje - else granom
			// ako je interval i [0, 1] to znaci da su obe grane dostizne; tu nam unija igra igru!
			
			if(!i->is_bottom(0) || !i->is_top(0)){
				outs() << "Fatal error. This shouldnt happen. Aborting all.\n";
				exit(1);
			}
			if(i->get_bottom(0)==0 && i->get_top(0)==0){  // Print information of confidence
				outs() << "Basic block: \""<< binst->getSuccessor(0)->getName() << "\" unreachable.\n";  
				outs() << "Basic block: \""<< binst->getSuccessor(1)->getName() << "\" will be reached.\n";
			}else if(i->get_bottom(0)==0 && i->get_top(0)==1){
				outs() << "Basic block: \"" << binst->getSuccessor(0)->getName() << "\" and \"" << binst->getSuccessor(1)->getName() << "\" both still reachable.\n";
				
				::if_branch =  binst->getSuccessor(1)->getName(); 
				
			}else{
				outs() << "Fatal error. This shouldnt happen. Aborting all.\n";
				exit(1);
			}
		}else{
			outs() << "Error. This sholudnt happen.\n";
		}

		outs() << "DONE: br.\n";
		return nullptr;
	}	

	IntervalOfConf* findByName(std::string name){
		outs() << "Searching for: " << name << "\n";
		for(auto it = _intervals.rbegin(); it != _intervals.rend(); ++it) // choose up to date data; reversed traverse of list
		{	
			if(it->second!=nullptr){
				if(it->second->get_name()==name)
					return it->second;
			}
		}
		outs()<< "Search unsuccessfull\n";
		return nullptr;	
	}

	// Variables initialization
	std::queue<Value*> init(Function& F)
	{
		// Iterate through function arguments
		auto fnIterator = F.arg_begin();
		
		outs() << "Functions argument initialization: \n";

		while (fnIterator != F.arg_end())
		{
			Value* arg = fnIterator;
			outs() << "[arg]: "<< arg << "\n*arg: "<<*arg<<"\n";
			IntervalOfConf* result = _parseInterval(arg);  // Parse Value* and put it into intervals
		  _intervals.push_back(std::make_pair(arg, result));

			fnIterator++;
		}
		
		std::queue<Value*> lista_intervala;
		outs() << "\nInitialization: \n\n";
		
		
		// Go through cfg
		for (auto& BB : F)
		{
		  outs() << "++++++++++++++++++++++++++++++++++\n";  // Fine print.
			outs() << "Basic block name: " << BB.getName() << "\n";
			for (auto& I : BB)
			{
				outs()<<"\n";
				outs() << "[I]: "<< I << "\n[opcode]: "<<I.getOpcode()<<"\n";
				Value* vred = cast<Value>(&I);
				
				IntervalOfConf* result = _parseInterval(vred);

			  _intervals.push_back(std::make_pair(vred, result));
				if(result!=nullptr)
					result->print();
        lista_intervala.push(vred);
			}	
		  outs() << "++++++++++++++++++++++++++++++++++\n";
		}
		outs()<<"\nEnd of initialization. \n";
		return lista_intervala;
	}
};

char AIProlaz::ID = 42;

// Pass register
// First false: I want to look at cfg of program.
// Second false: Analysis of program pass.
static RegisterPass<AIProlaz> X("AI-PROLAZ", "Apstraktna interpretacija", false, false);
