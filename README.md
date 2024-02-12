# Mersul Trenurilor

(**Proiect pentru materia Rețele de Calculatoare**)

Mersul trenurilor reprezintă o aplicație de tip server-client, pentru furnizarea de informații referitoare la circulația trenurilor, dar și actualizare a datelor referitoare la eventuale abateri.

Clientul poate trimite următoarele comenzi:
- **afisare_mersul_trenurilor** = solicitare de date privitoare la circulația trenurilor în ziua respectivă;
- **plecari_ultima_ora** = solicitare de date privitoare la plecările ce se vor întâmpla din gară în următoarea oră;
- **sosiri_ultima_ora** = solicitare de date privitoare la sosirile ce se vor întâmpla din gară în următoarea oră;
- **actualizare** = comandă pentru modificarea datelor privitoare la circulația trenurilor. Numai persoanele autorizate pot accesa această comanda (prin furnizarea parolei). Acestea sunt:
  - **INT** = întârziere de *n* minute (minutele sunt transmise odata cu comanda **INT**);  
  - **DEV** = mai devreme cu *n* minute (minutele sunt transmise odata cu comanda **DEV**);  
  - **CCP** = conform cu planul;
- **plecari_spre** = solicitare de date privitoare la trenuri ce merg spre destinatia furnizată;
- **sosiri_dinspre** = solicitare de date privitoare la trenuri ce ajung în gară din stația furnizată;
- **end_connex** = închiderea conexiunii dintre client și server

Baza de date din spate o reprezintă un fișier de tip *xml* cu informații privitoare la circulația trenurilor din ziua respectivă.
