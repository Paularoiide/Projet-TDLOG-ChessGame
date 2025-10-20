import tkinter as tk



window = tk.Tk()

# label = tk.Label(window, text="test")
# label.pack()

frame = tk.Frame(window, borderwidth=2, relief = tk.GROOVE)
frame.pack(side = tk.RIGHT, padx=10, pady=10)

tk.Label(frame, text="Test").pack(padx=10, pady=10)



window.mainloop()