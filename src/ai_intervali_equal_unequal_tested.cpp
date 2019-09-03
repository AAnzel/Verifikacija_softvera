#include <unordered_map>
#include <queue>
#include <string>
#include <list>

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

// Define class that represent interval of confidence for temporary variable
// Assusme that intervals are compact, in general [a, b], where a and b are integers or +/- inf.
class IntervalOfConf{
	private: 
		std::string name;		
		llvm::APInt bottom, top;
		int leftinf=1, rightinf=1;  // Indicators of infinity
	public:
		IntervalOfConf();
		IntervalOfConf(std::string);
		IntervalOfConf(std::string, const llvm::APInt, const llvm::APInt);

		void set_name(std::string);
		void set_value(const llvm::APInt);
		void set_bottom(const llvm::APInt);
		void set_top(const llvm::APInt);
	  void set_both(const llvm::APInt, const llvm::APInt);
		bool is_bottom();
		bool is_top();

		llvm::APInt get_top();
		llvm::APInt get_bottom();
		std::string get_name();

		void print();
};
IntervalOfConf::IntervalOfConf(){
	name = "-";
}
IntervalOfConf::IntervalOfConf(std::string _name){
	name = _name;
}
IntervalOfConf::IntervalOfConf(std::string _name, const llvm::APInt _bottom, const llvm::APInt _top){
	name = _name;
	bottom = _bottom; leftinf = 0;
	top = _top; rightinf = 0;
}
void IntervalOfConf::set_name(std::string _name){name = _name; }
void IntervalOfConf::set_value(const llvm::APInt _value){bottom = top = _value; leftinf = rightinf = 0; }
void IntervalOfConf::set_bottom(const llvm::APInt _bottom){bottom = _bottom; leftinf = 0; }
void IntervalOfConf::set_top(const llvm::APInt _top){top=_top; rightinf = 0; }
void IntervalOfConf::set_both(const llvm::APInt _bottom, const llvm::APInt _top){bottom = _bottom; top=_top; leftinf = rightinf = 0;}
bool IntervalOfConf::is_bottom(){return leftinf!=1; }
bool IntervalOfConf::is_top(){return rightinf!=1; }

std::string IntervalOfConf::get_name(){return name; }
llvm::APInt IntervalOfConf::get_top(){return top; }
llvm::APInt IntervalOfConf::get_bottom(){return bottom; }
void IntervalOfConf::print(){
	outs()<<name<<" in [";
	if(leftinf==1)
		outs()<<"-inf, ";
	else
		outs()<<bottom<<", ";
	if(rightinf==1)
		outs()<<"+inf]\n";
	else
		outs()<<top<<"]\n";
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
		if(j!=nullptr){
			if(j->is_bottom())
				i->set_bottom(j->get_bottom());
			if(j->is_top())
				i->set_top(j->get_top());
		}else{
			// Nothing, i is in [-inf, +inf] anywhay
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
		assert(v2);	

		IntervalOfConf* i = new IntervalOfConf();
		i->set_name(v2->getName());

		if(v1->getName()==""){
			// store from const int
		  Instruction *_inst2 = dyn_cast<Instruction>(v2);
		
			if (auto* konst_int = dyn_cast<ConstantInt>(v1))
			{
				i->set_value(konst_int->getValue());
			}else{
				outs() << "STORE ERROR!\n";
			}
		}else{
			// store from virtual register
			std::string name = v1->getName(); 
			IntervalOfConf* j = findByName(name);

			if(j!=nullptr){
				if(j->is_bottom())
					i->set_bottom(j->get_bottom());
				if(j->is_top())
					i->set_top(j->get_top());
			}else{
				// Nothing, i is in [-inf, +inf] anywhay
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
		if(i!=nullptr){
			if(j->is_bottom())
				i->set_bottom(j->get_bottom());
			if(j->is_top())
				i->set_top(j->get_top());
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
		
		IntervalOfConf* i = new IntervalOfConf(inst->getName());	
		IntervalOfConf* j = findByName(v1->getName());
		IntervalOfConf* k = findByName(v2->getName());
		if(j!=nullptr && k!=nullptr){  // [3, 6] + [4, 7] = [7, 13]
			if(j->is_bottom() && k->is_bottom())
				i->set_bottom(j->get_bottom()+k->get_bottom());
			if(j->is_top() && k->is_top())
				i->set_top(j->get_top()+k->get_top());
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
		if(j!=nullptr && k!=nullptr){  // j-k: [3, 6] - [4, 7] = [-4, 2] [3-7, 6-4]
			if(j->is_bottom() && k->is_top())
				i->set_bottom(j->get_bottom()-k->get_top());
			if(j->is_top() && k->is_bottom())
				i->set_top(j->get_top()-k->get_bottom());
		}else{
			// nothing. Result is unknown if I don't know any of j or k (i=j+k)
		}

		outs() << "DONE: sub.\n";
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
		if(auto* konst_int = dyn_cast<ConstantInt>(v2)){
			if(cast<Instruction>(v1)->getOpcode()==30){
				// %tmp < 5
				// *inst: "%tmp3 = icmp slt i32 %tmp2, 5"
				auto* konst_int = dyn_cast<ConstantInt>(v2);
		    assert(konst_int); 
				llvm::APInt B = konst_int->getValue();  // Border

				l1 = findByName(v1->getName());
		    l2 = findByName((cast<Instruction>(v1)->getOperand(0))->getName());  // l1 is loaded from l2; change both of them
				
				CmpInst *cp = dyn_cast<CmpInst>(vred);
		    assert(cp);
			  if(cp->getPredicate()==CmpInst::ICMP_SLT){  // newly splitted (date: 03.09.2019.)
					// slt <
					if(!l1->is_top())
						l1->set_top(B);
					else{
						if(l1->get_top().sge(B))
							l1->set_top(B);
					}
					if(!l2->is_top())
						l2->set_top(B);
					else{
						if(l2->get_top().sge(B))
							l2->set_top(B);	
					}
				}else if(cp->getPredicate()==CmpInst::ICMP_SLE){
					// sle <=
					B = B + 1;
					if(!l1->is_top())
						l1->set_top(B);
					else{
						if(l1->get_top().sge(B))
							l1->set_top(B);
					}
					if(!l2->is_top())
						l2->set_top(B);
					else{
						if(l2->get_top().sge(B))
							l2->set_top(B);	
					}
			}else if(cp->getPredicate()==CmpInst::ICMP_SGT){  // newly added 
					// sgt, "tmp>7"
					if(!l1->is_bottom())
						l1->set_bottom(B);
					else{
						if(l1->get_bottom().sle(B))
							l1->set_bottom(B);
					}
					if(!l2->is_bottom())
						l2->set_bottom(B);
					else{
						if(l2->get_bottom().sle(B))
							l2->set_bottom(B);	
					}
			}else if(cp->getPredicate()==CmpInst::ICMP_SGE){
					// sge, "tmp>=7"
					B -= 1;
					if(!l1->is_bottom())
						l1->set_bottom(B);
					else{
						if(l1->get_bottom().sle(B))
							l1->set_bottom(B);
					}
					if(!l2->is_bottom())
						l2->set_bottom(B);
					else{
						if(l2->get_bottom().sle(B))
							l2->set_bottom(B);	
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
					if(!l1->is_bottom())
						l1->set_bottom(B);
					else{
						if(l1->get_bottom().sle(B))
							l1->set_bottom(B);
					}
					if(!l2->is_bottom())
						l2->set_bottom(B);
					else{
						if(l2->get_bottom().sle(B))
							l2->set_bottom(B);	
					}
				}else if(cp->getPredicate()==CmpInst::ICMP_SLE){
					// sle, <=; "5 <= %tmp"  [4, +inf] 4 not included
					B -= 1;
					if(!l1->is_bottom())
						l1->set_bottom(B);
					else{
						if(l1->get_bottom().sle(B))
							l1->set_bottom(B);
					}
					if(!l2->is_bottom())
						l2->set_bottom(B);
					else{
						if(l2->get_bottom().sle(B))
							l2->set_bottom(B);	
					}
				}else if(cp->getPredicate()==CmpInst::ICMP_SGT){  // newly added 
					// sgt, >; "5 > %tmp"  [-inf, 5], 5 not included
					if(!l1->is_top())
						l1->set_top(B);
					else{
						if(l1->get_top().sge(B))
							l1->set_top(B);
					}
					if(!l2->is_top())
						l2->set_top(B);
					else{
						if(l2->get_top().sge(B))
							l2->set_top(B);	
					}
				}else if(cp->getPredicate()==CmpInst::ICMP_SGE){
					// sge, >=; "5 >= %tmp"  [-inf, 6], 6 not included
					B += 1;
					if(!l1->is_top())
						l1->set_top(B);
					else{
						if(l1->get_top().sge(B))
							l1->set_top(B);
					}
					if(!l2->is_top())
						l2->set_top(B);
					else{
						if(l2->get_top().sge(B))
							l2->set_top(B);	
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
		if(l1->is_bottom() && l1->is_top() && l1->get_bottom().sgt(l1->get_top())){
			outs()<<"Assertion failed in interval analysis time.\n";
			i->set_both(llvm::APInt(32, 0, true), llvm::APInt(32, 0, true));  // result is false, aka. [0, 0]
		//}else if(l1->is_bottom() && l1->is_top() && l1->get_bottom().sge(l1->get_top()) &&  l1->get_bottom().sle(l1->get_top())){ TODO: Add check when interval is only one number
		//	outs()<<"Assertion is satisfied, for shure.\n";
		//	i->set_both(llvm::APInt(32, 1, true), llvm::APInt(32, 1, true));  // result is true, aka. [1, 1]
		}else{
			outs()<<"Assertion could be satisfied.\n";
			i->set_both(llvm::APInt(32, 0, true), llvm::APInt(32, 1, true));  // result is possible true, aka. [0, 1]
		}
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
			if(j->is_bottom())
				r->set_bottom(j->get_bottom());
			if(j->is_top())
				r->set_top(j->get_top());
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
		if(b==1){
			// br label %bb6
			// Ok nothing to do, it will come anywhay
		}else if(b==2){
			// "br i1 %tmp3, label %bb4, label %bb5"

			IntervalOfConf* i = findByName(inst->getOperand(0)->getName());  // it should be [0, 0] (false first branch) or [0, 1] (possible both)
			if(!i->is_bottom() || !i->is_top()){
				outs() << "Fatal error. This shouldnt happen. Aborting all.\n";
				exit(1);
			}
			if(i->get_bottom()==0 && i->get_top()==0){  // Print information of confidence
				outs() << "Basic block: \""<< binst->getSuccessor(0)->getName() << "\" unreachable.\n";  
				outs() << "Basic block: \""<< binst->getSuccessor(1)->getName() << "\" will be reached.\n";
			}else if(i->get_bottom()==0 && i->get_top()==1){
				outs() << "Basich block: \"" << binst->getSuccessor(0)->getName() << "\" and \"" << binst->getSuccessor(1)->getName() << "\" both still reachable.\n";
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
