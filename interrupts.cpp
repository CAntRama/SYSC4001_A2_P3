/**
 *
 * @file interrupts.cpp
 * 
 * @author Khadija Lahlou
 * @author Casey 
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

    // to check again... (depending on what we are using, finish at the end) !!!!
    int current_time = 0;          //tracks the current time of the simulation
    
    bool in_kernel_mode = false;   //tracks if we are in kernel mode or user mode
    int context_save_time = 5;     //time it takes to save the context
    int context_restore_time = 5;  //time it takes to restore the context

    // Capital variables are constants

    
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
            int device = value;
            output << current_time << ", " << value << ", CPU burst\n";
            current_time += value;
        }

        else if(activity == "SYSCALL") {
            int device = value;
            int delay = device_table[device];
            string isr = vector_table[device];
            int position = device * vector_entry_size;

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
        }

        else if (activity == "END_IO") {
           
        }

        // Unknown activity
        else {
            std::cerr << "Error: Unknown activity type: " << activity << std::endl;
       
        }        
        

        /************************************************************************/

    }
    //Test
    input_file.close();

    write_output(execution);

    return 0;
}
