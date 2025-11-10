# Asus Zenbook Duo

## ASUS Zenbook Duo UX8406CA-PZ068X 

### Status

<table border="1">
<tr><th>Part     <th>Interface<th>Function                                <td>Status                      <td>Remarks</tr>
<tr><td>backlight<td>usb      <td>set level                               <td>working (pogo pin and wired)<td></tr>
<tr><td>         <td>         <td>switch backlight off if display is blank<td>working                     <td></tr>
<tr><td>         <td>bt       <td>set level                               <td>working                     <td></tr>
<tr><td>display  <td>usb      <td>switch off on pogo pin                  <td>working                     <td>requires root access</tr>
</table>

### Roadmap
  - correct lower touch display	 
  - observe operation
    - long sleep bluetooth kbd will not light (observe connection state necessary?)
    - SIGUSR1 not working with bluetooth

### parameter

start it with root access

```
  --level <num>     set level (0-3)
  --daemon          start as daemon
  --nodpms          don't check for screen blanking
```

### signals

```
	SUGUSR1         increment backlight level (0 .. 3)
    SIGUSR2         dump variables (NYI)
```
