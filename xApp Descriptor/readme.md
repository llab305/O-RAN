# 1. xApp Descriptor
The xApp descriptor is required for the xApp deploying within the Near-RT RIC, which includes the information on the xApp configuration in the JSON format.

# 2. xApp Descriptor Field
***
### 2.1 xapp_name (Mandatory)
  *	Writer : xApp designer
  *	Data type : string
  *	Description : The xApp name will provide a unique identifier for the xApp. </br>
    
    The name will be filled according to the rules of <b>"xapp-\<type\>-\<identifier\>"</b>.
    
    Because the name of this field will be the same as field <b> 2.3.1 name (container name) </b>, the total length is limited to 63 characters or less.
    
    Naming rules have the following definitions: </br>
    
    "\<type\>" identifies xApp's purpose. When filling in the content, the xApp designer must only use lowercase English letters and digits, and the content should be filled in according to the rules suggested in Table 2.1-1.</br>
    
    <p align="center"> Table 2.1-1 "< type >" suggested rules to fill in the corresponding table </p>
    
    |"\<type\>"|Description|
    |---|---|
    |"5qi-\<5QI value\>"|Identifies that the appropriate 5G QoS is assigned to traffic. "< 5QI value>" denotes QoS requirements for a specific vertical application, including packet error rate (PER), packet delay budget (PDB), throughput, and other conditions.|  
    |"handover"|Identifies that the UE hands over from a cell served by the base station to a cell served by another base station or the UE hands over from a cell served by the base station to another cell served by the base station.|
    
    "\<identifier\>" is used to distinguish xApps for the same purpose but with different algorithm designs or control different UEs. When filling in the content, the xApp designer must only use lowercase English letters and digits may use to fill content.
  
  *	Example: </br>
    if "\<type\>" is equal to "5qi-\<5QI value\>", as shown in Figure 2.1-1; if "\<type\>" is equal to "handover", as shown in Figure 2.1-2.
    
    <p align="center"> Figure 2.1-1 xapp_name - if "< type >" equals "5QI-<5QI value>" - example </p>
    ![xApp Descriptor/Figure](https://github.com/llab305/O-RAN/raw/main/xApp%20Descriptor/Figure/Figure%202.1-1.png)
    
    <p align="center"> Figure 2.1-2 xapp_name - if "< type >" equals "handover" - example </p>
    ![xApp Descriptor/Figure](https://github.com/llab305/O-RAN/raw/main/xApp%20Descriptor/Figure/Figure%202.1-2.png)

     
