⚙️ Funcționalitate

Programul funcționează în următorii pași:
1. 📥 Citirea fișierului

    Procesul cu rank 0 afișează lista de fișiere disponibile.

    Utilizatorul selectează fișierul dorit (ex: data_1000000.txt, data_9000000.txt).

    rank 0 citește toate datele în memorie.

2. 🔀 Distribuirea datelor (MPI Scatter)

    Datele sunt împărțite uniform între procese folosind MPI_Scatter.

    Fiecare proces primește o bucată din vectorul original și lucrează independent.

📌 Notă: versiunea actuală folosește MPI_Scatter, care poate pierde elemente dacă vectorul nu este divizibil exact. Pentru distribuție completă, se recomandă MPI_Scatterv.
3. 🧮 Sortare locală (paralelă)

    Fiecare proces aplică mergeSort() asupra propriului subset de date.

    Această parte este complet paralelă, fără comunicare între procese.

4. 🌲 Îmbinare paralelă (Merge Tree)

    După sortare, procesele își trimit rezultatele către alte procese, urmând o structură de arbore binar.

    La fiecare pas, datele sortate sunt combinate cu merge() și transmise mai departe.

    După log2(n) pași, procesul rank 0 are vectorul complet sortat.

5. 💾 Scrierea rezultatului

    rank 0 salvează vectorul sortat într-un fișier cu sufixul _sorted_parallel.txt.
