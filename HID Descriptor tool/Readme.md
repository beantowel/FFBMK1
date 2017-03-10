# HID-PID Descriptor tool
an extendable hid descriptor tool,parses the report descriptor written by natural language.
# Files
* \rptDsc:  
YAML syntax definition for _Sublime Text 3_ to highlight the report descriptor written in natural language. In Windows copy this to "C:\Users\Administrator\AppData\Roaming\Sublime Text 3\Packages\User", then choose syntax "rptDsc" when editing with _Sublime Text 3_.
* HID\_PID\_Descriptor\_Definitions.py:  
Human Interface Device & Physical Interface Device usage definitions.
* HID\_PID\_DescriptorGenTool.py:  
parses the .rptDsc into bytes(report descriptor).
* HID\_PID\_DataStructureGen.py:  
parses the .rptDsc into c data structure definitions.  
# How to Use
Before writing report descriptor in _HID\_Descriptor\_Input.rptDsc_, you are recommended to copy \rptDsc to the path refered above so that syntax highlight can work. To parse the descriptor into bytes run _DescriptorGenTool_, to parse into c data structure run _DataStructureGen_. 