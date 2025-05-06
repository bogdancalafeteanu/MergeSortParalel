#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <math.h>
#include <chrono>

using namespace std;

void merge(int* result, int* left, int leftSize, int* right, int rightSize) {
    int i = 0, j = 0, k = 0;
    while (i < leftSize && j < rightSize) {
        result[k++] = (left[i] <= right[j]) ? left[i++] : right[j++];
    }
    while (i < leftSize) result[k++] = left[i++];
    while (j < rightSize) result[k++] = right[j++];
}

void mergeSort(int* arr, int size) {
    if (size < 2) return;
    int mid = size / 2;
    int* left = (int*)malloc(mid * sizeof(int));
    int* right = (int*)malloc((size - mid) * sizeof(int));
    for (int i = 0; i < mid; i++) left[i] = arr[i];
    for (int i = mid; i < size; i++) right[i - mid] = arr[i];
    mergeSort(left, mid);
    mergeSort(right, size - mid);
    merge(arr, left, mid, right, size - mid);
    free(left); free(right);
}

void writeToFile(const char* filename, int* data, int size) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "w");
    if (err != 0 || !file) {
        printf("Eroare la scrierea in fisierul %s\n", filename);
        return;
    }
    for (int i = 0; i < size; i++) fprintf(file, "%d ", data[i]);
    fclose(file);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<string> input_files = {
        "data_1000000.txt", "data_2500000.txt", "data_5000000.txt",
        "data_7500000.txt", "data_9000000.txt"
    };

    int selected_file_index = 0;
    if (rank == 0) {
        cout << "Selecteaza un fisier pentru sortare:\n";
        for (size_t i = 0; i < input_files.size(); ++i)
            cout << i + 1 << ". " << input_files[i] << "\n";
        cout << "Introdu numarul fisierului: ";
        cin >> selected_file_index;

        if (selected_file_index < 1 || selected_file_index > input_files.size()) {
            cerr << "Selectie invalida!\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        selected_file_index--;
    }

    MPI_Bcast(&selected_file_index, 1, MPI_INT, 0, MPI_COMM_WORLD);
    const string& selected_file = input_files[selected_file_index];

    int total_elements = 0;
    if (selected_file.find("1000000") != string::npos) total_elements = 1000000;
    else if (selected_file.find("2500000") != string::npos) total_elements = 2500000;
    else if (selected_file.find("5000000") != string::npos) total_elements = 5000000;
    else if (selected_file.find("7500000") != string::npos) total_elements = 7500000;
    else if (selected_file.find("9000000") != string::npos) total_elements = 9000000;
    else {
        if (rank == 0) cerr << "Dimensiunea fisierului nu este recunoscuta.\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int chunk_size = total_elements / size;
    int* local_data = (int*)malloc(chunk_size * sizeof(int));
    int* full_data = nullptr;

    if (rank == 0) {
        full_data = (int*)malloc(total_elements * sizeof(int));
        FILE* input;
        errno_t err = fopen_s(&input, selected_file.c_str(), "r");
        if (err != 0 || !input) {
            printf("Nu s-a putut deschide fisierul %s\n", selected_file.c_str());
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < total_elements; ++i)
            if (fscanf_s(input, "%d", &full_data[i]) != 1) {
                printf("Eroare la citirea datelor.\n");
                fclose(input);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        fclose(input);
    }

    MPI_Scatter(full_data, chunk_size, MPI_INT, local_data, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    auto start = chrono::high_resolution_clock::now();
    mergeSort(local_data, chunk_size);

    int current_size = chunk_size;
    int step = 1;

    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int recv_size;
                MPI_Recv(&recv_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int* recv_data = (int*)malloc(recv_size * sizeof(int));
                MPI_Recv(recv_data, recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int* merged = (int*)malloc((current_size + recv_size) * sizeof(int));
                merge(merged, local_data, current_size, recv_data, recv_size);
                free(local_data);
                free(recv_data);
                local_data = merged;
                current_size += recv_size;
            }
        }
        else {
            int target = rank - step;
            MPI_Send(&current_size, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
            MPI_Send(local_data, current_size, MPI_INT, target, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    auto end = chrono::high_resolution_clock::now();
    double duration = chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.0;

    if (rank == 0) {
        printf("Timpul total (sortare + merge paralel): %.2f secunde\n", duration);
        string output_file = selected_file + "_sorted_parallel.txt";
        writeToFile(output_file.c_str(), local_data, total_elements);
        printf("Vectorul sortat a fost salvat in %s\n", output_file.c_str());
        free(full_data);
        free(local_data);
    }
    else {
        free(local_data);
    }

    MPI_Finalize();
    return 0;
}
