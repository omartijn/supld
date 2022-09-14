# supld
A daemon implementing the SUPL protocol for assisted GPS

## Purpose of this software

Acquiring a cold fix in an unobstructed environment with a GPS receiver
can take up to 12.5 minutes, since the receiver will need to download
the entire almanac and the data rate from the GPS satellites is slow.

For this reason the SUPL protocol (Secure User Plane Location) was created.
With this protocol, the phone sends local cell tower ids to a server using
the data connection. The server then looks up the location of the cell
tower, checks which satellites will be in view and sends the almanac data
back to the receiver. Since the data rate is much higher, a fix can be
acquired within seconds.

## Overview of functionality

To implement this service we will need the following things:

- A database of cell towers and their position

We can use Mozilla Location Services for this. They host all the data freely
available on their website and the data format is clearly documented.

- A running model of the GPS satellites and their almanac

Looks like we can get RINEX data from NASA. You need an account with them
to get at the data. I've not checked if it can be downloaded non-interactively.
I cannot find clear documentation on the format, nor on how we can use it to
generate and update almanacs for each of the satellites.

- Understand the protocol used between the receiver and the SUPL server

The protocol uses ASN.1 messages to communicate. The message formats can
be extracted from [Open Mobile Alliance](https://www.openmobilealliance.org/release/SUPL/).
