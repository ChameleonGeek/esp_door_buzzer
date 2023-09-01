# ESP Door Buzzer
This project will provide easier access to an electrically locked door, allowing patrons of a shared space to enter the space without needing constant member intervention.  

It will use an ESP32 which connects to the primary network inside the space.  The ESP32 will also operate as an access point configured as a captive portal. The captive portal page will present different options depending upon the status of the person who connects to the access point.

This is not intended to be high level security.  It is intended to offer a moderate level of filtering access to the site, with responsible staff working at the space performing final determination as to whether the individual granted access by the ESP is permitted in the space.

This is not intended to manage 100% of the access control at the space.  It is intended to reduce, not eliminate, the frequency in which staff is required to stop their activities and open the door or manually buzz in patrons.  Clearly, if the cell network is slow to respond, the email-to-text function may take too long for the patrons to want to wait, at which time they can knock on the door to request access.

**NOTE:**  I am aware that using the MAC address is probably not a viable option, as modern smartphones are implementing rolling MAC addresses to prevent unauthorized tracking of the phone and therefore its owner.  Patrons may need to enter a unique username instead of have the ESP use the phone's MAC Address.

### Registration:
When the user first connects to the Access Point, the ESP will read the MAC Address of the device that connected.  The ESP will use an API call over the primary network to determine if the MAC Address is already registered.  If not, the user will be prompted to enter their name and phone number.  The captive portal page will POST the entered information and store MAC, Name and phone number.

### Authorization:
Staff who work at the space and are authorized to do so will link the MAC address and name to the membership system.  This link will allow a check for the patron's permissions to enter once the user connects to the Access Point at the start of later visits.

### "Knocking on the door:"
During later visits to the space, the registered patron will connect to the ESP Access Point again.  The ESP will use the MAC Address to identify the patron and text the patron a one-time code.  Once the patron receives the one-time code, they enter it into the web form and the ESP will trigger the electronic door lock.  The one-time code will expire once it is used, or after a timeframe to be specified later.

## TO DO/TO CONSIDER:
Solidify the captive portal process.  In initial attempts, the captive portal opens the portal page automatically on a Windows PC, but not on an Android phone.

Since multiple devices can be connected to the ESP AP at the same time, find a way to ensure that the ESP knows definitively which device (by MAC or username) is actively being communicated with.  Failure to do so can cause sending the 2FA code to the wrong phone number.
