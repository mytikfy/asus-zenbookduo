# Asus Zenbook Duo

## ASUS Zenbook Duo UX8406CA-PZ068X 

### Status

<table border="1">
<tr><th>Part     <th>Interface<th>Function                                <td>Status                      <td>Remarks</tr>
<tr><td>backlight<td>usb      <td>set level                               <td>working (pogo pin and wired)<td></tr>
<tr><td>         <td>         <td>switch backlight off if display is blank<td>working                     <td></tr>
<tr><td>         <td>bt       <td>set level                               <td>working                     <td></tr>
<tr><td>display  <td>usb      <td>switch off on pogo pin                  <td>working                     <td>requires root permissions</tr>
</table>

### Roadmap
  - correct lower touch display	 
  - observe operation
    - SIGUSR1 not working with bluetooth

### parameter

(start it as root)

```
  --level <num>			set level (0-3)
  --daemon				start as daemon
  --nodpms				don't check for screen blanking
  --timeout <number>	wakeup timeout in ms
  --version				just show the version and quit (shorthand: -v)
```

### signals

```
	SUGUSR1         increment backlight level (0 .. 3) rotating
	SIGUSR2         dump variables/states (NYI)
```
