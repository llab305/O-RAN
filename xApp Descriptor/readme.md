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
    
    * "\<type\>" identifies xApp's purpose. When filling in the content, the xApp designer must only use lowercase English letters and digits, and the content should be filled in according to the rules suggested in Table 2.1-1.</br>
    
    <p align="center"> Table 2.1-1 "< type >" suggested rules to fill in the corresponding table </p>
    
    |"\<type\>"|Description|
    |---|---|
    |"5qi-\<5QI value\>"|Identifies that the appropriate 5G QoS is assigned to traffic. "<5QI value>" denotes QoS requirements for a specific vertical application, including packet error rate (PER), packet delay budget (PDB), throughput, and other conditions.|  
    |"handover"|Identifies that the UE hands over from a cell served by the base station to a cell served by another base station or the UE hands over from a cell served by the base station to another cell served by the base station.|
    
    * "\<identifier\>" is used to distinguish xApps for the same purpose but with different algorithm designs or control different UEs. When filling in the content, the xApp designer must only use lowercase English letters and digits may use to fill content.
  
  *	Example: if "\<type\>" is equal to "5qi-\<5QI value\>", as shown in Figure 2.1-1; if "\<type\>" is equal to "handover", as shown in Figure 2.1-2.
  
    <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.1-1.png"> </image></p>
    <p align="center"> Figure 2.1-1 xapp_name - if "< type >" equals "5QI-<5QI value>" - example </p>
    
    <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.1-2.png"></image></p>
    <p align="center"> Figure 2.1-2 xapp_name - if "< type >" equals "handover" - example </p>
    
***
### 2.2 version (Mandatory)    
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The version number of xApp descriptor. </br> The naming will be filled in according to the rules of "\<major version\>.\<minor version\>.\<patch version\>", where "\<major version\>" increases the version number sequentially from 1, "\<minor version\>" and "\<patch version\>" increase the version number sequentially starting from 0, that is to say, all version number updates after this version number are based on the modified content of this xApp descriptor. </br></br> The rules for incrementing the version number are defined as follows: </br> 
     1. If the xApp descriptor changes the main field, it will increment "\<major version\>", at which time "\<minor version\>" and "\<patch version\>" must be reset to zero. Main fields e.g., <b> xapp_name (Mandatory) field, version (Mandatory) field, containers (Mandatory) field, and messaging (Optional) field</b>.</br> 
     2. If xApp descriptor maintains a backward-compatible main field and adds a secondary field, "\<minor version\>" is incremented, and "\<patch version\>" must be reset to zero when incremented. The secondary fields are shown in Table 2.2-1.</br>
     
     <p align="center"> Table 2.2-1 Secondary field correspondence table under the main field </p>

    |secondary field\main field|containers (Mandatory)|messaging (Optional)|
    |---|---|---|
    |1|name (Mandatory) (container name)|ports (Mandatory)|  
    |2|image (Mandatory)|name (Mandatory) (port name)|
    |3|registry (Mandatory)|container (Mandatory)|  
    |4|handover|port (Mandatory)|
    |5|name 	(Mandatory) (image name)|txMessages (Mandatory)|  
    |6|handover|rxMessages (Mandatory)|
    |7|tag (Mandatory)|policies (Mandatory)|  
    |8||PolicyTypeId (Conditional)|
    |9||SupportXapp (Optional)|  
    |10||description (Optional)|  

    3. If the xApp descriptor maintains a backward-compatible main field and no secondary fields are added, but the content needs to be corrected, increment "\<patch version\>". 
     
     Naming rules have the following definitions:
     * "\<major version\>" is the major version number of the xApp descriptor. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     * "\<minor version\>" is the minor version number of xApp descriptor. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     * "\<patch version\>" is the patch number of the xApp descriptor. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     
 *	Example : Figure 2.2-1 </br>
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.2-1.png"></image></p>
 <p align="center"> Figure 2.2-1 version - example </p>
  
***


### 2.3	containers (Mandatory)
It covers the container the xApp will run in. However, this version limits xApp to only be deployed as a Pod, a Pod can only run one container, and a container can only support one image, but an image can be referenced by multiple Pods. For containers, include the following fields: <b>name (Mandatory), image (Mandatory)</b>.

***
#### 2.3.1 name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : In the name of the container, fill in the same fields as <b>xapp_name (Mandatory)</b>.</br>
 *	Example : Same as Figure 2.1-2, as shown in Figure 2.3-1.
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-1.png"></image></p>
  <p align="center"> Figure 2.3-1 containers.name - example </p>
  
***

#### 2.3.2 image (Mandatory) 
Contains the image in the container that will run this xApp. For image, include the following fields: <b>registry (Mandatory), name 	(Mandatory), and tag (Mandatory)</b>.

***
##### 2.3.2.1	registry (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : Since the xApp designer cannot obtain the IP and port number of the local private image repository, "NULL" must be filled in, and then handed over to the xApp Manager to fill in the content.</br>
 *	Example : Figure 2.3-2
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-2.png"></image></p>
 <p align="center"> Figure 2.3-2 containers.image.registry - example </p>
 
***

##### 2.3.2.2	name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : </br>
 *	Example : 
***

     
     
     
     
