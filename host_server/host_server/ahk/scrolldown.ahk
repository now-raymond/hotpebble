scrolldown(){
	MouseGetPos, x, y
	y2 := y+30
	Send {Mbutton Down}
	MouseMove, %x%, %y2%
	Sleep 3000
	Send {Mbutton up}
	MouseMove, %x%, %y%
	return
}
scrolldown()