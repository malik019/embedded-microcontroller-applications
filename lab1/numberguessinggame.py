import random as r


def randGen():
    return r.randint(1, 100)

def leesInvoer(msg):
    return input(msg)

#game loop
while (True):

    pogingen = 2
    gen = randGen() 
    gewonnen = False
    print("=== Number Gussing Game ===")
    print("Raad het getal tussen 1 en 100")

    try:
        while(pogingen):
        
            getal = int(leesInvoer("Geef Getal? "))        
            while getal > 100 or getal < 1:
                getal = int(leesInvoer("Geef Getal? "))

            print(f"Je hebt {pogingen} pogingen!")
            print(f"Jouw gok: {getal}")
            if getal == gen:
                gewonnen = True
                break            
            elif getal > gen:
                print("Te hoog!")
            elif getal < gen:
                print("Te laag!")

            print("\n")
            pogingen -=1

        if gewonnen:
            print("Proficiat je heb gewonnen!!!")
        else:
            print("Verloren")

        start = str(leesInvoer("Opnieuw spellen ja/nee? ").strip()).upper()
        while start not in ['JA', 'NEE']:
            start = str(leesInvoer("Opnieuw spellen ja/nee? ").strip()).upper()
        print(start)
        if (start == "NEE"):
            break
        print("\n")
    except ValueError:
        print("Ongeldig invoer")





            

    
        
        

            
            

    
    