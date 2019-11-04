# Mashcontrol
This is a remote operation tool for managing the Linux operating system. You can use it to remote manage the host which has no pulic ip address. In many cases it is needed.

## How to install ?<br>
The code was so simple. you can compile and run it directly.
* 1 Download the source code . <br>
`git clone https://github.com/pingshunbobo/Mashcontrol.git`

* 2 Run make to compile directly.<br>
`make -C Mashcontrol`

* 3 Run the Server at the server host.<br>
`~/MashControl/Server/serv`

* 4 Run the Client at the client host as daemond.<br>
`~/MashControl/Client/client`


##Control terminal usage
* Run the Control at the server host.<br>
**`~/MashControl/Control/control`** <br>
`[mashcmd]`

* Display all the client connected to the server. <br>
`[mashcmd]`**`display`** <br>
`--id: 6, role: client, addess: 192.168.141.129 @public-ip` <br>

* Select client as interface.<br>
`[mashcmd]`**`select 6`**<br>
`[mashcmd]`**`display`** <br>
`++id: 6, role: client, addess: 192.168.141.129 @public-ip` <br>

* Enter the interface of selected client.<br>
`[mashcmd]`**`interface`**<br>
`[mashcmd][root@server-1 Client]#`<br>
* **Now you can run command on the terminal same as the client host.**

* Input **`exit`** on bash you can logout bash and back to mashcmd%. 

##know issues

##Have more questions?
Send email to pingshunbobo@sina.cn
