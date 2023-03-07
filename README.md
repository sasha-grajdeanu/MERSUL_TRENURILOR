# Laboratorul nr. 3

## Compulsory

1. Create an object-oriented model of the problem. You should have at least the following classes Person, Company. (solved) (Am creat clasele *person* si *company*, fiecare avand atributul *name*, constructorul specific, getters si setters, unde am dat override la metoda *getName*)
2. Both classes should implement the interface java.util.Comparable. The natural order of the objects will be given by their names.(solved) (Am creat metoda *compareTo* careia i-am dat override pentru a compara doua nume)
3. Create the interface Node that defines the method used to obtain the name of a person or company. The classes above must implement this interface.(solved) (Am creat interfata *node*, pe care am implementat-o pentru clasele *person* si *company* cu metoda *getName*)
4. Create a java.util.List containing node objects and print it on the screen. (solved) (Am creat o lista de noduri, sortata cu ajutorul functiei *collection.sort*, parametrii fiind lista de noduri si o functie lambda pentru sortarea nodurilor dupa nume)
