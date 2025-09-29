#!/bin/bash
job_folder=$1
file_name=$2

echo "job_folder: $job_folder"
echo "file_name: $file_name"   
# region #### Createing Jobfolder Directory if doesn't exist
if [ ! -d "$job_folder" ]; then
    echo "Creating folder $job_folder"
    mkdir "$job_folder"
fi
# endregion

# region #### Here we compile the files. <y/n>
while true; do
    read -p "Do you want to compile files? (y/n or m for move): " choice
    if [ "$choice" == "y" ]; then
        echo "Compiling files....."
        # g++ -O3 -fopenmp $file_name.cpp -o $file_name -Wfatal-errors
        # g++ -O3 -fopenmp -I./../include/ $file_name.cpp -o $file_name -Wfatal-errors
        if g++ -O3 -fopenmp -I./../include/ $file_name.cpp -o $file_name -Wfatal-errors; then
            echo "File compiled successfully."
        else
            echo "Compilation failed."
            exit 1
        fi
        echo "File compiled successfully."
    fi

    if [ "$choice" == "y" ] || [ "$choice" == "m" ]; then
        mv $file_name "$job_folder"
        echo "Compiled file $file_name moved to $job_folder."
        break
    elif [ "$choice" == "n" ]; then
        echo "Compilation skipped."
        break
    else
        echo "Invalid choice. Please enter 'y' or 'n'."
    fi
done
# endregion
