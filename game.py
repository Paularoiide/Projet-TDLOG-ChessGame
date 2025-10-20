from tkinter import *



menu = Tk()

menu.title("Chess Prime, the best chess game !")
menu.geometry('1000x700')


frame = Frame(menu, borderwidth=20, relief = GROOVE)
frame.pack(padx=90, pady=9)

Label(frame, text="Test").pack(padx=10, pady=10)

Label(frame, text = "Shall we play a game?").pack()




menu.mainloop()