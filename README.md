# ESP Door Buzzer
This project is still in the early steps of development.  Any code currently attached to this repository will be unstable and at best only partly functional.  This ReadMe document will also be changed to more clearly specify plans and functionality of the project.

A primary goal of the project is to provide a solution which requires as little instruction and training as possible.  If training is required to use it, most targeted users will opt out of using it.

This project will provide easier access to an electrically locked door, allowing patrons of a shared space to enter the space without needing constant member intervention.  While the interaction with this project *may* be performed with a laptop or tablet, the target audience will predominantly use Android or iOS phones.

It will use an ESP32 which connects to the primary wireless network inside the space.  The ESP32 will also operate as an access point configured as a captive portal. The captive portal page will present different options depending upon the status of the person who connects to the access point.

This is not intended to be high level security.  It is intended to offer a moderate level of filtering access to the site, with responsible staff working at the space performing final determinations as to whether the individual granted access by the ESP is permitted to remain in the space.

This is not intended to manage 100% of the access control at the space.  It is intended to reduce, not eliminate, the frequency in which staff is required to stop their other activities and open the door or manually buzz in patrons.  Since the project relies on external services, outages or delays in these services will impact QoS - at which time patrons can revert to status quo and knock on the door to request access.

Since the goal of this project is to make life easier for staff and patrons alike, this project uses functions and software the patrons already frequently use.  Some tools which offer a far greater level of security have been suggested, but each requires patrons to install or access software they don't already use.  A goal of this project is to provide access control to the space *and* require as close to zero instruction as possible.  If training is needed for average users, this project will be too complicated to be used by most patrons, which will make it fail its primary goal.

**NOTE:**  I am aware that using the MAC address is probably not a viable option, as modern smartphones are implementing changing MAC addresses to prevent unauthorized tracking of the phone and therefore its owner.  Patrons may need to enter a unique username instead of have the ESP use the phone's MAC Address.

### Registration:
When the user first connects to the Access Point, the ESP will read the MAC Address of the device that connected.  The ESP will use an API call over the primary network to determine if the MAC Address is already registered.  If not, the user will be prompted to enter their name and phone number.  The captive portal page will POST the entered information and store MAC, Name and phone number.

### Authorization:
Staff who work at the space and are authorized to do so will link the MAC address and name to the membership system.  This link will allow a check for the patron's permissions to enter once the user connects to the Access Point at the start of subsequent visits.

### "Knocking on the door:"
During later visits to the space, the registered patron will connect to the ESP Access Point again.  The ESP will use the MAC Address to identify the patron and text the patron a one-time code.  Once the patron receives the one-time code, they enter it into the web form and the ESP will trigger the electronic door lock.  The one-time code will expire once it is used, or after a timeframe to be specified later.

## TO DO/TO CONSIDER:
Solidify the captive portal process.  In initial attempts, the captive portal opens the portal page automatically on a Windows PC, but not on an Android phone.  No testing has been performed on iOS devices or Linux.

While the MAC Address may not be a viable key to identify the user over time, it appears it will be essential during the short time interactions with the ESP.

Since multiple devices can be connected to the ESP AP at the same time, find a way to ensure that the ESP knows definitively which device (by MAC or username) is actively being communicated with.  Failure to do so can cause sending the 2FA code to the wrong phone number.

HTTPS may be sufficiently secure to bypass the 2FA requirement, letting the user simply enter username and password on the captive portal page.  Determine how certificates are handled in the process, which may exceed the minimal instruction/training goal of this project.
