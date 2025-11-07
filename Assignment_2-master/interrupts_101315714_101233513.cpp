/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @author Casey Ramanamapanoharana 101233513
 * @author Khadija Lahlou 
 *
 */

#include "interrupts_101315714_101233513.hpp"

#define CONTEXT_SAVE_EXECUTION_TIME 10 // in ms
#define ISR_EXECUTION_TIME 40 // in ms
#define IRET_EXECUTION_TIME 1 // in ms
#define NOT_DEVICE -1

std::tuple<std::string, std::string, int> simulate_trace(std::vector<std::string> trace_file, int time, std::vector<std::string> vectors, std::vector<int> delays, std::vector<external_file> external_files, PCB current, std::vector<PCB> wait_queue) {

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output
    std::string system_status = "";  //!< string to accumulate the system status output
    int current_time = time;
    int current_device = NOT_DEVICE;
    static int pid_identifier = 1;

    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        auto [activity, duration_intr, program_name] = parse_trace(trace);

        if(activity == "CPU") { //As per Assignment 1
            // Context:
            //   CPU, 51 <-- We are here
            //   SYSCALL, 14
            //   CPU, 39
            //   END_IO, 14
            //
            // Added IRET to make it obvious the return to CPU
            // This is the first CPU instruction --> Will be considered as a CPU Burst can be initialize_variable() or calculate(x) or delete (y,x)
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";

                current_time += duration_intr;

            continue;
        } else if(activity == "SYSCALL") { //As per Assignment 1
            // Context: 
            //   CPU, 51
            //   SYSCALL, 14 <-- We are here
            //   CPU, 39
            //   END_IO, 14
            //  
            // Device delays --> Total from ISR, +40 every step, from example execution file from github 2 ISR :
            // transfer data from device to memory, check for errors
            current_device = duration_intr;

            auto [boilerplate_execution, new_current_time] = intr_boilerplate(current_time, current_device, CONTEXT_SAVE_EXECUTION_TIME, vectors);
            execution += boilerplate_execution;

            current_time = new_current_time;

            execution += std::to_string(current_time) + ", " + std::to_string(ISR_EXECUTION_TIME) + ", SYSCALL: run the ISR (device driver)\n";
            current_time += ISR_EXECUTION_TIME;
            
            execution += std::to_string(current_time) + ", " + std::to_string(ISR_EXECUTION_TIME) + ", transfer data from device to memory\n";
            current_time += ISR_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(delays.at(current_device)-(2*ISR_EXECUTION_TIME)) + ", check for errors\n";
            current_time += delays.at(current_device)-(2*ISR_EXECUTION_TIME);

            execution += std::to_string(current_time) + ", " + std::to_string(IRET_EXECUTION_TIME) + ", IRET \n";
            current_time += IRET_EXECUTION_TIME;

            continue;
        } else if(activity == "END_IO") {
            // Context: 
            //   SYSCALL, 14
            //   CPU, 39
            //   END_IO, 14 <-- We are here
            //
            // Added IRET to make it obvious the return to CPU
            // Device delays --> Total from ISR, +40 every step, from example execution file from github 1 ISR : check device status
            current_device = duration_intr;

            auto [boilerplate_execution, new_current_time] = intr_boilerplate(current_time, current_device, CONTEXT_SAVE_EXECUTION_TIME, vectors);
            execution += boilerplate_execution;

            current_time = new_current_time;

            execution += std::to_string(current_time) + ", " + std::to_string(ISR_EXECUTION_TIME) + ", end of I/O " + std::to_string(current_device) + ": run the ISR (device driver)\n";
            current_time += ISR_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(delays.at(current_device)-ISR_EXECUTION_TIME) + ", check device status\n";
            current_time += delays.at(current_device)-ISR_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(IRET_EXECUTION_TIME) + ", IRET \n";
            current_time += IRET_EXECUTION_TIME;

            current_device = NOT_DEVICE;

        } else if(activity == "FORK") {
            // Context:
            //    FORK, 10 <-- We are here
            //    IF_CHILD, 0
            //    EXEC program1, 50 //child executes program1
            //    IF_PARENT, 0
            //    EXEC program2, 25 //parent executes program2
            //    ENDIF, 0
            //
            // Parent is copied = makes child --> partition allocated to child as exact copy of parent apart from PID (and mem. address) then child process is ran (Parent enters wait())
            auto [intr, time] = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr;
            current_time = time;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your FORK output here

            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", cloning to PCB\n";
            current_time += duration_intr;

            PCB child(pid_identifier++,current.PID,current.program_name,current.size,current.partition_number);   // <--- important changes start here (copy of process)
            
            if(!allocate_memory(&child)) { // initial mem. allocation of child as copy of parent
                std::cerr << "ERROR! Memory allocation failed!" << std::endl;

                child.partition_number = -1; //<-- child was not allocated space
            }

            wait_queue.push_back(current); // parent enters wait

            execution += std::to_string(current_time) + ", " + std::to_string(0) + ", scheduler called\n";
            current_time += 0;

            execution += std::to_string(current_time) + ", " + std::to_string(IRET_EXECUTION_TIME) + ", IRET \n";
            current_time += IRET_EXECUTION_TIME;

            system_status += "time:" +std::to_string(current_time)+ "; current trace: " + trace + "\n";
            system_status += print_PCB(child,wait_queue); 

            ///////////////////////////////////////////////////////////////////////////////////////////

            //The following loop helps you do 2 things:
            // * Collect the trace of the child (and only the child, skip parent)
            // * Get the index of where the parent is supposed to start executing from
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_flag = false;
            int parent_index = 0;

            for(size_t j = i; j < trace_file.size(); j++) {
                auto [_activity, _duration, _pn] = parse_trace(trace_file[j]);
                if(skip && _activity == "IF_CHILD") {
                    skip = false;
                    continue;
                } else if(_activity == "IF_PARENT"){
                    skip = true;
                    parent_index = j;
                    if(exec_flag) {
                        break;
                    }
                } else if(skip && _activity == "ENDIF") {
                    skip = false;
                    continue;
                } else if(!skip && _activity == "EXEC") {
                    skip = true;
                    child_trace.push_back(trace_file[j]);
                    exec_flag = true;
                }

                if(!skip) {
                    child_trace.push_back(trace_file[j]);
                }
            }
            i = parent_index;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the child's trace, run the child (HINT: think recursion)

            auto [child_execution, child_system_status, child_time] = 
            simulate_trace(
                    child_trace, 
                    current_time, 
                    vectors, 
                    delays,
                    external_files, 
                    child, 
                    wait_queue
            );

            execution += child_execution;
            system_status+= child_system_status;
            current_time = child_time;
            

            if (!wait_queue.empty()) //<-- No more children
            {
                current = wait_queue.back(); //<--parents is back to running
                wait_queue.pop_back();
            }

            ///////////////////////////////////////////////////////////////////////////////////////////


        } else if(activity == "EXEC") {
            // Context:
            //    FORK, 
            //    IF_CHILD, 0
            //    EXEC program1, 50 //child executes program1 <-- We are somewhere here ish
            //    IF_PARENT, 0
            //    EXEC program2, 25 //parent executes program2
            //    ENDIF, 0
            //
            // Changing the content of child to be the actual program and not the parent copy anymore
            auto [intr, time] = intr_boilerplate(current_time, 3, 10, vectors);
            current_time = time;
            execution += intr;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your EXEC output here

            unsigned int size_new_exec = get_size(program_name,external_files);

            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", Program is"+ std::to_string(size_new_exec)+ "Mb large" + "\n";  //<--- want a new program different from parent so need to find space
            current_time += duration_intr;

            execution += std::to_string(current_time) + ", " + std::to_string(size_new_exec*15) + ", loading program into memory" + "\n";
            current_time += size_new_exec*15;

            if (current.partition_number != -1){ //<-- free old memory not using anymore
                free_memory(&current);
            }

            execution += std::to_string(current_time) + ", 3, marking partition as occupied\n";
            current_time += 3;

            execution += std::to_string(current_time) + ", 6, updating PCB\n";  
            current_time += 6;

            std::string old_program_name = current.program_name;
            unsigned int old_size = current.size;

            current.program_name = program_name; //<--update the pcb with the new program info
            current.size = size_new_exec;

            if (!allocate_memory(&current)){
                std::cerr << "ERROR! Memory allocation failed!" << std::endl;
                execution += std::to_string(current_time)+", 0, Error memory not allocated for "+program_name+"\n"; //<-- extra but wanted to test when program too big and memory not allocated

                current.program_name = old_program_name; //<-- goes back to old program
                current.size = old_size;

                continue; //<-- just ends up skipping current exec
            }

            execution += std::to_string(current_time) + ", 0, scheduler called\n";
            current_time += 0;

            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

            system_status += "time:" +std::to_string(current_time)+ "; current trace: " + trace + "\n";
            system_status += print_PCB(current,wait_queue);

            ///////////////////////////////////////////////////////////////////////////////////////////


            std::ifstream exec_trace_file(program_name + ".txt");

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the exec's trace (i.e. trace of external program), run the exec (HINT: think recursion)

            auto [exec_execution, exec_system_status, exec_time] = 
            simulate_trace(
                    exec_traces, 
                    current_time, 
                    vectors, 
                    delays,
                    external_files, 
                    current, 
                    wait_queue
            );

            execution += exec_execution;
            system_status += exec_system_status;
            current_time = exec_time;

            ///////////////////////////////////////////////////////////////////////////////////////////

            break; //Why is this important? (answer in report)

        }
    }

    return {execution, system_status, current_time};
}

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //external_files is a C++ std::vector of the struct 'external_file'. Check the struct in 
    //interrupt.hpp to know more.
    auto [vectors, delays, external_files] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    //Just a sanity check to know what files you have
    print_external_files(external_files);

    //Make initial PCB (notice how partition is not assigned yet)
    PCB current(0, -1, "init", 1, -1);
    //Update memory (partition is assigned here, you must implement this function)
    if(!allocate_memory(&current)) {
        std::cerr << "ERROR! Memory allocation failed!" << std::endl;
    }

    std::vector<PCB> wait_queue;

    /******************ADD YOUR VARIABLES HERE*************************/


    /******************************************************************/

    //Converting the trace file into a vector of strings.
    std::vector<std::string> trace_file;
    std::string trace;
    while(std::getline(input_file, trace)) {
        trace_file.push_back(trace);
    }

    auto [execution, system_status, _] = simulate_trace(   trace_file, 
                                            0, 
                                            vectors, 
                                            delays,
                                            external_files, 
                                            current, 
                                            wait_queue);

    input_file.close();

    write_output(execution, "execution.txt");
    write_output(system_status, "system_status.txt");

    return 0;
}
