# Plant sensor waardes
grond_vocht = 35      # percentage
temperatuur = 22      # graden Celsius
licht_niveau = 1200   # lux

# Gezonde waardes voor deze plant:
# Grond vocht: tussen 30% en 65%
# Temperatuur: tussen 18°C en 24°C
# Licht: minimaal 1000 lux



# Opdrachten:
# 1. Check elke waarde en sla op of deze OK is (True/False)
# 2. Als alle waardes OK zijn: print "Plant is gezond"
# 3. Anders: print welke waardes aangepast moeten worden

# Schrijf hier je code:
import time as t
is_grond_vocht = False
is_temperatuur = False
is_light_niveau = False


if grond_vocht >= 30 and grond_vocht <= 65:
    is_grond_vocht = True
else:
    is_grond_vocht = False

if temperatuur >= 18 and temperatuur <= 24:
    is_temperatuur = True
else:
    is_temperatuur = False

if licht_niveau >1000:
    is_light_niveau = True
else:
    is_light_niveau= False

while (1):
    if is_grond_vocht and is_light_niveau and is_temperatuur:
        print("Pant is gezond")
    else:
        if not is_grond_vocht:
            print(f"Grond Vochtheid: {grond_vocht} moet aangepast worden")
        if not is_temperatuur:
            print(f"Tempatuur: {temperatuur} moet aangepast worden")
        if not is_light_niveau:
            print(f"Licht Niveau: {licht_niveau} moet aangepast worden")
    t.sleep(5)

