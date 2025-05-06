# MergeSortParalel

Acest program implementează sortarea paralelă a unui vector de numere întregi folosind:
MPI (Message Passing Interface) – pentru distribuirea datelor între mai multe procese,
algoritmul Merge Sort – pentru sortarea locală și combinarea datelor.

Distribuția muncii între procese :
-Procesul cu rank 0 citeste intregul fisier de intrare și determină câte elemente trebuie sortate.
-Vectorul de date este împartit egal în chunk_size = total_elements / num_procs.
-Fiecare proces primeste o parte din vector cu MPI_Scatter, deci sortarea se face în paralel pe bucati independente.
