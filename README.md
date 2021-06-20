# Tema 1 Protocoale de Comunicatii

    Student: Stancioiu Laura Ioana
    Grupa: 322CD

## **Parsare tabela de rutare**

### Preprocesare

* Am citit linie cu linie din fisierul primit ca argument
* Am retinut intrarile din tabela de rutare intr-un vector de structuri de tip route_table_entry.
* Am sortat vectorul crescator dupa prefix (daca doua intrari au acelasi prefix sunt sortate crescator dupa masca)  
folosind qsort (complexitate O(nlogn)).

### Gasirea rutei optime

* Functia get_best_route primeste adresa destinatie si intoarce ruta cea mai buna.
* Pentru a evita o cautare liniara folosesc binary search pe vectorul sortat de structuri.
* Ca sa iau in calcul gasirea celei mai restrictive rute (cu masca cea mai mare) se retine la fiecare pas  
indexul elementului cu masca cea mai mare, ce satisface conditia, iar algoritmul se termina  
cand nu mai sunt elemente in vector.
* Complexitatea este O(logn)

## **Etape forwarding**

### ICMP echo request

* Verific daca ip destinatie pentru pachetul primit este acelasi cu cel al interfetei pe care a venit pachetul.
* In acest caz inseamna ca pachetul a ajuns la destinatie (routerul) si ii pot da drop.
* Singurul caz in care acest lucru nu se intampla este daca pachetul este de tip ICMP ECHO REQUEST  
    caz in care trebuie trimis inapoi sursei un ICMP ECHO REPLY.
* Fiind un packet ICMP cu code 0, campurile id si sequence pot fi 0 sau se pot  
    pastra aceleasi [valori de la sender](https://tools.ietf.org/html/rfc792).

### ARP request

* Daca pachetul este de tip arp request si are adresa ip destinatie cea a interfetei pe care a venit,  
 routerul trimite inapoi sursei un arp reply cu adresa MAC a interfetei.
* Se da drop la pachet.

### ARP reply

* Se verifica daca exista pachete in coada de pachete ce trebuie trimise (pending_packets).
* Daca da, se updateaza adresa MAC destinatie a primului pachet din coada  
cu adresa MAC a senderului pachetului de arp reply si se trimite pachetul.
* Se adauga in tabela arp (un vector de structuri de tip arp_entry) corespondenta  
  dintre ip-ul senderului pachetului ARP si adresa sa MAC.
* Se da drop la pachet.

### __Daca pachetul nu s-a incadrat in niciuna dintre categoriile precedente inseamna ca routerul trebuie sa il trimita mai departe.__

***

### Time Exceeded

* Se verifica ttl-ul pachetului si daca este <= 1 se trimite sursei un pachet  
de tip ICMP TIME EXCEEDED si se da drop la pachet.

### ttl & checksum

* Se verifica checksum-ul si daca este gresit se da drop la pachet.
* In caz contrar:
  * ttl-ul se decrementeaza
  * checksum-ul se recalculeaza.

### Best route

* Se cauta in tabela de rutare pentru destinatia urmatoare a pachetului.
* Daca nu se gaseste nicio ruta potrivita se trimite sursei un pachet ICMP de  
tip DESTINATION UNREACHABLE si se da drop la pachet.
* Daca ruta a fost gasita se actualizeaza interfata pachetului cu cea a rutei gasite  
si se modifica adresa MAC sursa.

### No arp entry

* Se cauta in tabela arp adresa MAC destinatie pentru next hop.
* In cazul in care aceasta nu exista se trimite un pachet ARP REQUEST cu adresa MAC  
destinatie de tip broadcast(ff:ff:ff:ff:ff:ff) pentru a o afla, avand ca ip destinatie  
adresa ip a lui next hop.
* Pachetul este bagat in coada.
* Se trece la urmatoarea iteratie.

### Send packet

* Daca a fost gasita adresa MAC se updateaza adresa MAC destinatie si se trimite pachetul.
