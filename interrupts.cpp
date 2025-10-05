/**
 *
 * @file interrupts.cpp
 * 
 * @author Khadija Lahlou
 * @author Casey Ramanampanoharana
 * @date 2024-10-03
 * 
 */

#include<interrupts.hpp>

// quick note: if you get an error here
// if it fails #include "interrupts.hpp"
// to run ./interrupts test /test/ <your_vector_table.txt> <your_device_table.txt>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    // initialize array of CPU operations
    std::array<std::string, 2> cpu_operations = {"calculate(x)", "delete(y, x)"}; 
    
    // Constants
    int CONTEXT_SAVE_EXECUTION_TIME = 10;    // Time to save context
    int IRET_EXECUTION_TIME = 1;             // Time for IRET
    int NOT_DEVICE = -1;                     // Indicator for no device

    int current_time = 0;          // Tracks the current time of the simulation

    bool variables_initialized = false;   // Tracks if we are in kernel mode or user mode
    bool within_syscall = false;          // Tracks if we are within a syscall
   
    int current_device = NOT_DEVICE;      // Time it takes to save the context
    int next_cpu_operation = 0;           // Next Operation to perform by CPU
    
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/

        // small note:
        // '<<' means append to string
        // '>>' means read from string

        // go see part of where it says simulation in instructions
        // basically in this section we write whats happening 
        // and add time of each steps to current_time variable

        if (activity == "CPU"){
            
            if (variables_initialized == false){
                // Context:
                //   CPU, 51 <-- We are here
                //   SYSCALL, 14
                //
                // This is the first CPU instruction in the file and we assume that it's the initialize_variabes()
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst()\n";

                variables_initialized = true;

                current_time += duration_intr;
            }
            
            else if(within_syscall){
                // Context:
                //   SYSCALL, 14
                //   CPU, 39 <-- We are here
                //   END_IO, 14
                //
                // What is this CPU instruction exactly?
                //   1. Is this the "Calling device driver?"
                //   2. Is the duration the time that the driver took to execute?    
                //   3. Is it something else?
                //
                // In the example:
                //   595, 24, call device driver // include all activities of the ISR (takes 110ms according to device table)
                //   ...
                //   705, 1, IRET // IRET after 110ms
                // 
                //   1. What is 24? Is the duration that comes wit the CPU instruction?
                //   2. If that is the case, why is the IRET at 705 (595+110) and not 729 (595+24+110)
                //
                // And does the "..." mean something?
                //  1. Are we supposed to fill it in with something?
                //     1. With what?
                //  2. Are we supposed to just write "..." as well?
                //  3. For now, we're not writing anything about it.
                
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", SYSCALL: run the ISR (device driver)\n";

                // For now, we're going to assume that:
                //   1. This CPU instruction is the "Calling device driver"
                //   2. Total duration = CPU duration + Device delay
                current_time += duration_intr + delays.at(current_device);

                /*
                // STEP 1: Switch to kernel mode (in order to have privileged access to memory)
                output << current_time << ", " << mode_switch_time << ", switch to kernel mode\n";
                current_time += mode_switch_time;  // add mode switch time to total time
                
                // STEP 2: Save/Restore Context (meaning: )
                output << current_time << ", " << context_time << ", context saved\n";
                current_time += context_time; // add context time to total time

                // STEP 3: Find the vector from the vector table (calculate the position of the address of ISR)
                output << current_time << ", 1, find vector " << device << " in memory position " << position << "\n";
                current_time += position; // add time to find the vector to total time

                // STEP 4: Obtain the ISR address from the vector table
                output << current_time << ", 1, obtain ISR address " << isr << "\n";
                current_time += isr;    // add time to obtain the ISR address to total time

                // STEP 5: Execute ISR body
                output << current_time << ", " << delay << ", call device driver\n";
                current_time += delay;  // add delay time to indicate time spent in ISR to total time

                // STEP 6: Execute IRET (Interrupt Return)
                output << current_time << ", 1, IRET\n" << iret << "\n";
                current_time += iret; // add IRET time to total time
                */
            }

            else {
                // Context:
                //   END_IO, 14
                //   CPU, 72 <-- We are here
                //
                // We're in one of the non initialize CPU operations: calculate or delete
                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", " + cpu_operations[next_cpu_operation] + "\n";

                next_cpu_operation++;
                if (next_cpu_operation >= cpu_operations.size())
                {
                    next_cpu_operation = 0;
                }

                current_time += duration_intr;
            }

            continue;
        }

        else if (activity == "SYSCALL"){
            // Context: 
            //   SYSCALL, 14 <-- We are here
            //   CPU, 39
            //   END_IO, 14
            current_device = duration_intr;

            auto [boilerplate_execution, new_current_time] = intr_boilerplate(current_time, current_device, CONTEXT_SAVE_EXECUTION_TIME, vectors);
            execution += boilerplate_execution;

            current_time = new_current_time;

            within_syscall = true;

            continue;
        }

        else if (activity == "END_IO") {
            // Context: 
            //   SYSCALL, 14
            //   CPU, 39
            //   END_IO, 14 <-- We are here
            execution += std::to_string(current_time) + ", " + std::to_string(IRET_EXECUTION_TIME) + ", IRET\n";
            current_time += IRET_EXECUTION_TIME;

            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", end of I/O " + std::to_string(current_device) + ": interrupt\n";
            current_time += duration_intr;

            within_syscall = false;
            current_device = NOT_DEVICE;

            continue;  
        }
     

        /************************************************************************/

    }
    // Test2
    input_file.close();

    write_output(execution);

    return 0;
}
