#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

bitset<32> OPCODE_MASK = 0x3F << 26;
bitset<32> RS_MASK = 0X1F << 21;
bitset<32> RT_MASK = 0X1F << 16;
bitset<32> RD_MASK = 0X1F << 11;
bitset<32> SHAMT_MASK = 0X1F << 6;
bitset<32> FUNCT_MASK = 0X3F;
bitset<32> IMMED_MASK = 0XFFFF;
bitset<32> ADDR_MASK = 0X3FFFFFF;


struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
    IFStruct(){
        nop = false;
    }
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
    IDStruct(){
        nop = true;
    }  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop; 
    EXStruct(){
        nop = true;
        wrt_enable = 0;
        alu_op = 1;
    } 
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;
    MEMStruct(){
        nop = true;
        wrt_enable = 0;
    }    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;  
    WBStruct(){
        nop = true;
    }   
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line.substr(0,8));
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line.substr(0,8));
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        { 
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 

int main()
{
    
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    struct stateStruct state;
    int cycle = 0;
    bool isBranch = false;
    bool FwdRs = false;
    bool FwdRt = false;
    bool isStall = false;
    bitset<32> RsFwd_data;
    bitset<32> RtFwd_data;
    bitset<32> Brachoffset;
			
             
    while (1) {
        struct stateStruct newState = state;

        /* --------------------- WB stage --------------------- */
        if(!state.WB.nop){
            if(state.WB.wrt_enable){
                
                myRF.writeRF(state.WB.Wrt_reg_addr,state.WB.Wrt_data);
            }
        }
        newState.WB.nop = state.MEM.nop;


        /* --------------------- MEM stage --------------------- */
        if(!state.MEM.nop){
            if(state.MEM.rd_mem){
                //lw
                myDataMem.readDataMem(state.MEM.ALUresult);
            }else if(state.MEM.wrt_mem){
                //sw
                myDataMem.writeDataMem(state.MEM.ALUresult,state.MEM.Store_data);
            }
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_data = state.MEM.rd_mem ? myDataMem.ReadData : state.MEM.ALUresult;
            newState.WB.wrt_enable = state.MEM.wrt_enable;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            
        }
        newState.MEM.nop = state.EX.nop;


        /* --------------------- EX stage --------------------- */
        if(!state.EX.nop){

            uint16_t MSB = state.EX.Imm[15];
            for(int i = 0; i< 16;i++){
                MSB |= (MSB << 1);
            }
            bitset<32> IMM = state.EX.Imm.to_ulong();
            bitset<32> sign_extend_op2 = (bitset<32>(MSB) << 16) | IMM;
            //MUX for ALU oprand2
            bitset<32> oprand2 = state.EX.is_I_type ? sign_extend_op2 : state.EX.Read_data2;
            bitset<32> oprand1 = state.EX.Read_data1.to_ulong();

            oprand1 = FwdRs ? RsFwd_data : oprand1;
            oprand2 = FwdRt ? RtFwd_data : oprand2;

            if(true){
                //ALU operation
                if(state.EX.alu_op == 1){ //lw sw addu
                    newState.MEM.ALUresult = oprand1.to_ulong() + oprand2.to_ulong();
                }else{
                    newState.MEM.ALUresult = oprand1.to_ulong() - oprand2.to_ulong();
                }

                newState.MEM.Store_data = state.EX.Read_data2;
                newState.MEM.Rs = state.EX.Rs;
                newState.MEM.Rt = state.EX.Rt;
                newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
                newState.MEM.rd_mem = state.EX.rd_mem;
                newState.MEM.wrt_mem = state.EX.wrt_mem;
                newState.MEM.wrt_enable = state.EX.wrt_enable;
            }
            
        }
        if(isStall){
            isStall = false;
        }
        newState.EX.nop = state.ID.nop;

        /* --------------------- ID stage --------------------- */
        if(!state.ID.nop){
            bitset<32> opcode = (state.ID.Instr & OPCODE_MASK) >> 26;
            bitset<32> functcode = state.ID.Instr & FUNCT_MASK;
            bitset<32> rs = (state.ID.Instr & RS_MASK) >> 21;  
            bitset<32> rt = (state.ID.Instr & RT_MASK) >> 16;
            bitset<32> rd = (state.ID.Instr & RD_MASK) >> 11;
            bitset<32> addr = state.ID.Instr & ADDR_MASK;

            if(state.EX.rd_mem && (state.EX.Rt == bitset<5>(rs.to_ulong()) || state.EX.Rt == bitset<5>(rt.to_ulong()))){
                isStall = true;
                newState.EX.nop = true;
            }
            
            if(!isStall){
                FwdRs = false;
                FwdRt = false;
                if(newState.MEM.wrt_enable && bitset<5>(rs.to_ulong()) == newState.MEM.Wrt_reg_addr && !state.EX.nop){
                    FwdRs = true;
                    RsFwd_data = newState.MEM.ALUresult;
                }else{
                    FwdRs = false;
                }
                if(newState.MEM.wrt_enable && bitset<5>(rt.to_ulong()) == newState.MEM.Wrt_reg_addr && !state.EX.nop){
                    FwdRt = true;
                    
                    RtFwd_data = newState.MEM.ALUresult;
                    
                }else{
                    FwdRt = false;
                }

                //MEM forwarding
                if(newState.WB.wrt_enable && newState.WB.Wrt_reg_addr == bitset<5>(rs.to_ulong())){
                    if(FwdRs == false){
                        FwdRs = true;
                        RsFwd_data = newState.WB.Wrt_data;
                    }
                }
                if(newState.WB.wrt_enable && newState.WB.Wrt_reg_addr == bitset<5>(rt.to_ulong())){
                    if(FwdRt == false){
                        FwdRt = true;
                        RtFwd_data = newState.WB.Wrt_data;
                    }
                    
                }
            }


            
                newState.EX.Read_data1 = myRF.readRF(rs.to_ulong());
                newState.EX.Read_data2 = myRF.readRF(rt.to_ulong());
                newState.EX.Imm = (state.ID.Instr & IMMED_MASK).to_ulong();
                newState.EX.Rs = rs.to_ulong();
                newState.EX.Rt = rt.to_ulong();
                newState.EX.is_I_type = (opcode != 0x0);
                newState.EX.Wrt_reg_addr = newState.EX.is_I_type ? rt.to_ulong() : rd.to_ulong();
                newState.EX.rd_mem = opcode == 0x23;  //set to 1 for lw
                newState.EX.wrt_mem = opcode == 0x2B; // set to 1 for sw
                newState.EX.wrt_enable = (opcode == 0x0) || (opcode == 0x23);
                newState.EX.alu_op = opcode == 0x0 && functcode == 0x23 ? 0 : 1;
            

            //beq(bne)
            if(opcode == 0x04){
                //Unstall and forwarding
                if(isStall){
                    bitset<5> op2 = myDataMem.readDataMem(newState.MEM.ALUresult).to_ulong();
                    cout << newState.MEM.Wrt_reg_addr.to_ulong() << endl;
                    cout << rt.to_ulong()<<"cycle  "<<cycle << endl;
                    if(newState.MEM.Wrt_reg_addr.to_ulong() == rt.to_ulong()){
                        isStall = false;
                        newState.EX.nop = false;
                        if(myRF.readRF(rt.to_ulong()).to_ulong() != op2.to_ulong()){
                            isBranch = true;
                            
                        }
                    }
                    
                }else{
                    // if beq has not dependencies
                    if(newState.EX.Read_data1 != newState.EX.Read_data2){
                        isBranch = true;
                    }
                }
            }
            
        }
        newState.ID.nop = state.IF.nop;

        if(isBranch){
            cout <<"oppps"<<endl;
            newState.ID.nop = true;
            uint16_t MSB = newState.EX.Imm[15];
                for(int i = 0; i< 16;i++){
                    MSB |= (MSB << 1);
                }
                bitset<32> IMM = newState.EX.Imm.to_ulong();
                bitset<32> sign_extend_op2 = (bitset<32>(MSB) << 16) | IMM;
                //newState.IF.PC = state.IF.PC.to_ulong() + (sign_extend_op2 << 2).to_ulong();
                Brachoffset = (sign_extend_op2 << 2).to_ulong();
        }

        /* --------------------- IF stage --------------------- */
        if(!state.IF.nop && !isStall){
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            if(!isBranch){
                //cout << "Not branch" << cycle << endl;
                if(newState.ID.Instr != 0xffffffff){
                    newState.IF.PC = state.IF.PC.to_ulong() + 4;
                }else{
                    newState.IF.nop = true;
                    newState.ID.nop = true;
                }
            }else if(isBranch){
                newState.IF.PC = state.IF.PC.to_ulong() + Brachoffset.to_ulong();
                //cout << newState.IF.PC.to_ulong()<<"cycle"<<cycle  << "PC  " << newState.IF.PC.to_ulong() <<endl;
                isBranch = false;
            }
        }

             
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 
        //cout <<state.IF.PC << endl;
        cycle++;  	
        //if(cycle > 9) break;
    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}