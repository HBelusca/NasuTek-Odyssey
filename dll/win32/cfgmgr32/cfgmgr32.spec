@ stdcall CMP_Init_Detection(long) setupapi.CMP_Init_Detection
@ stdcall CMP_RegisterNotification(ptr ptr long ptr) setupapi.CMP_RegisterNotification
@ stdcall CMP_Report_LogOn(long long) setupapi.CMP_Report_LogOn
@ stdcall CMP_UnregisterNotification(ptr) setupapi.CMP_UnregisterNotification
@ stdcall CMP_WaitNoPendingInstallEvents(long) setupapi.CMP_WaitNoPendingInstallEvents
@ stdcall CMP_WaitServicesAvailable(ptr) setupapi.CMP_WaitServicesAvailable
@ stdcall CM_Add_Empty_Log_Conf(ptr ptr long long) setupapi.CM_Add_Empty_Log_Conf
@ stdcall CM_Add_Empty_Log_Conf_Ex(ptr ptr long long ptr) setupapi.CM_Add_Empty_Log_Conf_Ex
@ stdcall CM_Add_IDA(ptr str long) setupapi.CM_Add_IDA
@ stdcall CM_Add_IDW(ptr wstr long) setupapi.CM_Add_IDW
@ stdcall CM_Add_ID_ExA(ptr str long ptr) setupapi.CM_Add_ID_ExA
@ stdcall CM_Add_ID_ExW(ptr wstr long ptr) setupapi.CM_Add_ID_ExW
@ stub CM_Add_Range # setupapi.CM_Add_Range
@ stdcall CM_Add_Res_Des(ptr ptr long ptr long long) setupapi.CM_Add_Res_Des
@ stdcall CM_Add_Res_Des_Ex(ptr ptr long ptr long long long) setupapi.CM_Add_Res_Des_Ex
@ stdcall CM_Connect_MachineA(str ptr) setupapi.CM_Connect_MachineA
@ stdcall CM_Connect_MachineW(wstr ptr) setupapi.CM_Connect_MachineW
@ stdcall CM_Create_DevNodeA(ptr str long long) setupapi.CM_Create_DevNodeA
@ stdcall CM_Create_DevNodeW(ptr wstr long long) setupapi.CM_Create_DevNodeW
@ stdcall CM_Create_DevNode_ExA(ptr str long long long) setupapi.CM_Create_DevNode_ExA
@ stdcall CM_Create_DevNode_ExW(ptr wstr long long long) setupapi.CM_Create_DevNode_ExW
@ stub CM_Create_Range_List # setupapi.CM_Create_Range_List
@ stdcall CM_Delete_Class_Key(ptr long) setupapi.CM_Delete_Class_Key
@ stdcall CM_Delete_Class_Key_Ex(ptr long long) setupapi.CM_Delete_Class_Key_Ex
@ stdcall CM_Delete_DevNode_Key(long long long) setupapi.CM_Delete_DevNode_Key
@ stdcall CM_Delete_DevNode_Key_Ex(long long long ptr) setupapi.CM_Delete_DevNode_Key_Ex
@ stub CM_Delete_Range # setupapi.CM_Delete_Range
@ stub CM_Detect_Resource_Conflict # setupapi.CM_Detect_Resource_Conflict
@ stub CM_Detect_Resource_Conflict_Ex # setupapi.CM_Detect_Resource_Conflict_Ex
@ stdcall CM_Disable_DevNode(long long) setupapi.CM_Disable_DevNode
@ stdcall CM_Disable_DevNode_Ex(long long ptr) setupapi.CM_Disable_DevNode_Ex
@ stdcall CM_Disconnect_Machine(long) setupapi.CM_Disconnect_Machine
@ stub CM_Dup_Range_List # setupapi.CM_Dup_Range_List
@ stdcall CM_Enable_DevNode(long long) setupapi.CM_Enable_DevNode
@ stdcall CM_Enable_DevNode_Ex(long long ptr) setupapi.CM_Enable_DevNode_Ex
@ stdcall CM_Enumerate_Classes(long ptr long) setupapi.CM_Enumerate_Classes
@ stdcall CM_Enumerate_Classes_Ex(long ptr long ptr) setupapi.CM_Enumerate_Classes_Ex
@ stdcall CM_Enumerate_EnumeratorsA(long str ptr long) setupapi.CM_Enumerate_EnumeratorsA
@ stdcall CM_Enumerate_EnumeratorsW(long wstr ptr long) setupapi.CM_Enumerate_EnumeratorsW
@ stdcall CM_Enumerate_Enumerators_ExA(long str ptr long long) setupapi.CM_Enumerate_Enumerators_ExA
@ stdcall CM_Enumerate_Enumerators_ExW(long wstr ptr long long) setupapi.CM_Enumerate_Enumerators_ExW
@ stub CM_Find_Range # setupapi.CM_Find_Range
@ stub CM_First_Range # setupapi.CM_First_Range
@ stdcall CM_Free_Log_Conf(ptr long) setupapi.CM_Free_Log_Conf
@ stdcall CM_Free_Log_Conf_Ex(ptr long ptr) setupapi.CM_Free_Log_Conf_Ex
@ stdcall CM_Free_Log_Conf_Handle(ptr) setupapi.CM_Free_Log_Conf_Handle
@ stub CM_Free_Range_List # setupapi.CM_Free_Range_List
@ stdcall CM_Free_Res_Des(ptr ptr long) setupapi.CM_Free_Res_Des
@ stdcall CM_Free_Res_Des_Ex(ptr ptr long long) setupapi.CM_Free_Res_Des_Ex
@ stdcall CM_Free_Res_Des_Handle(ptr) setupapi.CM_Free_Res_Des_Handle
@ stub CM_Free_Resource_Conflict_Handle # setupapi.CM_Free_Resource_Conflict_Handle
@ stdcall CM_Get_Child(ptr long long) setupapi.CM_Get_Child
@ stdcall CM_Get_Child_Ex(ptr long long long) setupapi.CM_Get_Child_Ex
@ stdcall CM_Get_Class_Key_NameA(ptr str ptr long) setupapi.CM_Get_Class_Key_NameA
@ stdcall CM_Get_Class_Key_NameW(ptr wstr ptr long) setupapi.CM_Get_Class_Key_NameW
@ stdcall CM_Get_Class_Key_Name_ExA(ptr str ptr long long) setupapi.CM_Get_Class_Key_Name_ExA
@ stdcall CM_Get_Class_Key_Name_ExW(ptr wstr ptr long long) setupapi.CM_Get_Class_Key_Name_ExW
@ stdcall CM_Get_Class_NameA(ptr str ptr long) setupapi.CM_Get_Class_NameA
@ stdcall CM_Get_Class_NameW(ptr wstr ptr long) setupapi.CM_Get_Class_NameW
@ stdcall CM_Get_Class_Name_ExA(ptr str ptr long long) setupapi.CM_Get_Class_Name_ExA
@ stdcall CM_Get_Class_Name_ExW(ptr wstr ptr long long) setupapi.CM_Get_Class_Name_ExW
@ stdcall CM_Get_Class_Registry_PropertyA(ptr long ptr ptr ptr long ptr) setupapi.CM_Get_Class_Registry_PropertyA
@ stdcall CM_Get_Class_Registry_PropertyW(ptr long ptr ptr ptr long ptr) setupapi.CM_Get_Class_Registry_PropertyW
@ stdcall CM_Get_Depth(ptr long long) setupapi.CM_Get_Depth
@ stdcall CM_Get_Depth_Ex(ptr long long long) setupapi.CM_Get_Depth_Ex
@ stdcall CM_Get_DevNode_Registry_PropertyA(long long ptr ptr ptr long) setupapi.CM_Get_DevNode_Registry_PropertyA
@ stdcall CM_Get_DevNode_Registry_PropertyW(long long ptr ptr ptr long) setupapi.CM_Get_DevNode_Registry_PropertyW
@ stdcall CM_Get_DevNode_Registry_Property_ExA(long long ptr ptr ptr long long) setupapi.CM_Get_DevNode_Registry_Property_ExA
@ stdcall CM_Get_DevNode_Registry_Property_ExW(long long ptr ptr ptr long long) setupapi.CM_Get_DevNode_Registry_Property_ExW
@ stdcall CM_Get_DevNode_Status(ptr ptr long long) setupapi.CM_Get_DevNode_Status
@ stdcall CM_Get_DevNode_Status_Ex(ptr ptr long long long) setupapi.CM_Get_DevNode_Status_Ex
@ stdcall CM_Get_Device_IDA(long str long long) setupapi.CM_Get_Device_IDA
@ stdcall CM_Get_Device_IDW(long wstr long long) setupapi.CM_Get_Device_IDW
@ stdcall CM_Get_Device_ID_ExA(long str long long long) setupapi.CM_Get_Device_ID_ExA
@ stdcall CM_Get_Device_ID_ExW(long wstr long long long) setupapi.CM_Get_Device_ID_ExW
@ stdcall CM_Get_Device_ID_ListA(str str long long) setupapi.CM_Get_Device_ID_ListA
@ stdcall CM_Get_Device_ID_ListW(wstr wstr long long) setupapi.CM_Get_Device_ID_ListW
@ stdcall CM_Get_Device_ID_List_ExA(str str long long long) setupapi.CM_Get_Device_ID_List_ExA
@ stdcall CM_Get_Device_ID_List_ExW(wstr wstr long long long) setupapi.CM_Get_Device_ID_List_ExW
@ stdcall CM_Get_Device_ID_List_SizeA(ptr str long) setupapi.CM_Get_Device_ID_List_SizeA
@ stdcall CM_Get_Device_ID_List_SizeW(ptr wstr long) setupapi.CM_Get_Device_ID_List_SizeW
@ stdcall CM_Get_Device_ID_List_Size_ExA(ptr str long long) setupapi.CM_Get_Device_ID_List_Size_ExA
@ stdcall CM_Get_Device_ID_List_Size_ExW(ptr wstr long long) setupapi.CM_Get_Device_ID_List_Size_ExA
@ stdcall CM_Get_Device_ID_Size(ptr long long) setupapi.CM_Get_Device_ID_Size
@ stdcall CM_Get_Device_ID_Size_Ex(ptr long long long) setupapi.CM_Get_Device_ID_Size_Ex
@ stdcall CM_Get_Device_Interface_AliasA(str ptr str ptr long) setupapi.CM_Get_Device_Interface_AliasA
@ stdcall CM_Get_Device_Interface_AliasW(wstr ptr wstr ptr long) setupapi.CM_Get_Device_Interface_AliasW
@ stdcall CM_Get_Device_Interface_Alias_ExA(str ptr str ptr long long) setupapi.CM_Get_Device_Interface_Alias_ExA
@ stdcall CM_Get_Device_Interface_Alias_ExW(wstr ptr wstr ptr long long) setupapi.CM_Get_Device_Interface_Alias_ExW
@ stdcall CM_Get_Device_Interface_ListA(ptr str str long long) setupapi.CM_Get_Device_Interface_ListA
@ stdcall CM_Get_Device_Interface_ListW(ptr wstr wstr long long) setupapi.CM_Get_Device_Interface_ListW
@ stdcall CM_Get_Device_Interface_List_ExA(ptr str str long long ptr) setupapi.CM_Get_Device_Interface_List_ExA
@ stdcall CM_Get_Device_Interface_List_ExW(ptr wstr wstr long long ptr) setupapi.CM_Get_Device_Interface_List_ExW
@ stdcall CM_Get_Device_Interface_List_SizeA(ptr ptr str long) setupapi.CM_Get_Device_Interface_List_SizeA
@ stdcall CM_Get_Device_Interface_List_SizeW(ptr ptr wstr long) setupapi.CM_Get_Device_Interface_List_SizeW
@ stdcall CM_Get_Device_Interface_List_Size_ExA(ptr ptr str long ptr) setupapi.CM_Get_Device_Interface_List_Size_ExA
@ stdcall CM_Get_Device_Interface_List_Size_ExW(ptr ptr wstr long ptr) setupapi.CM_Get_Device_Interface_List_Size_ExW
@ stdcall CM_Get_First_Log_Conf(ptr long long) setupapi.CM_Get_First_Log_Conf
@ stdcall CM_Get_First_Log_Conf_Ex(ptr long long long) setupapi.CM_Get_First_Log_Conf_Ex
@ stdcall CM_Get_Global_State(ptr long) setupapi.CM_Get_Global_State
@ stdcall CM_Get_Global_State_Ex(ptr long long) setupapi.CM_Get_Global_State_Ex
@ stdcall CM_Get_HW_Prof_FlagsA(str long ptr long) setupapi.CM_Get_HW_Prof_FlagsA
@ stdcall CM_Get_HW_Prof_FlagsW(wstr long ptr long) setupapi.CM_Get_HW_Prof_FlagsW
@ stdcall CM_Get_HW_Prof_Flags_ExA(str long ptr long long) setupapi.CM_Get_HW_Prof_Flags_ExA
@ stdcall CM_Get_HW_Prof_Flags_ExW(wstr long ptr long long) setupapi.CM_Get_HW_Prof_Flags_ExW
@ stdcall CM_Get_Hardware_Profile_InfoA(long ptr long) setupapi.CM_Get_Hardware_Profile_InfoA
@ stdcall CM_Get_Hardware_Profile_InfoW(long ptr long) setupapi.CM_Get_Hardware_Profile_InfoW
@ stdcall CM_Get_Hardware_Profile_Info_ExA(long ptr long long) setupapi.CM_Get_Hardware_Profile_Info_ExA
@ stdcall CM_Get_Hardware_Profile_Info_ExW(long ptr long long) setupapi.CM_Get_Hardware_Profile_Info_ExW
@ stdcall CM_Get_Log_Conf_Priority(ptr ptr long) setupapi.CM_Get_Log_Conf_Priority
@ stdcall CM_Get_Log_Conf_Priority_Ex(ptr ptr long long) setupapi.CM_Get_Log_Conf_Priority_Ex
@ stdcall CM_Get_Next_Log_Conf(ptr ptr long) setupapi.CM_Get_Next_Log_Conf
@ stdcall CM_Get_Next_Log_Conf_Ex(ptr ptr long long) setupapi.CM_Get_Next_Log_Conf_Ex
@ stdcall CM_Get_Next_Res_Des(ptr ptr long ptr long) setupapi.CM_Get_Next_Res_Des
@ stdcall CM_Get_Next_Res_Des_Ex(ptr ptr long ptr long long) setupapi.CM_Get_Next_Res_Des_Ex
@ stdcall CM_Get_Parent(ptr long long) setupapi.CM_Get_Parent
@ stdcall CM_Get_Parent_Ex(ptr long long long) setupapi.CM_Get_Parent_Ex
@ stdcall CM_Get_Res_Des_Data(ptr ptr long long) setupapi.CM_Get_Res_Des_Data
@ stdcall CM_Get_Res_Des_Data_Ex(ptr ptr long long long) setupapi.CM_Get_Res_Des_Data_Ex
@ stdcall CM_Get_Res_Des_Data_Size(ptr ptr long) setupapi.CM_Get_Res_Des_Data_Size
@ stdcall CM_Get_Res_Des_Data_Size_Ex(ptr ptr long long) setupapi.CM_Get_Res_Des_Data_Size_Ex
@ stub CM_Get_Resource_Conflict_Count # setupapi.CM_Get_Resource_Conflict_Count
@ stub CM_Get_Resource_Conflict_DetailsA # setupapi.CM_Get_Resource_Conflict_DetailsA
@ stub CM_Get_Resource_Conflict_DetailsW # setupapi.CM_Get_Resource_Conflict_DetailsW
@ stdcall CM_Get_Sibling(ptr long long) setupapi.CM_Get_Sibling
@ stdcall CM_Get_Sibling_Ex(ptr long long long) setupapi.CM_Get_Sibling_Ex
@ stdcall CM_Get_Version() setupapi.CM_Get_Version
@ stdcall CM_Get_Version_Ex(long) setupapi.CM_Get_Version_Ex
@ stub CM_Intersect_Range_List # setupapi.CM_Intersect_Range_List
@ stub CM_Invert_Range_List # setupapi.CM_Invert_Range_List
@ stdcall CM_Is_Dock_Station_Present(ptr) setupapi.CM_Is_Dock_Station_Present
@ stdcall CM_Is_Dock_Station_Present_Ex(ptr long) setupapi.CM_Is_Dock_Station_Present_Ex
@ stdcall CM_Locate_DevNodeA(ptr str long) setupapi.CM_Locate_DevNodeA
@ stdcall CM_Locate_DevNodeW(ptr wstr long) setupapi.CM_Locate_DevNodeW
@ stdcall CM_Locate_DevNode_ExA(ptr str long long) setupapi.CM_Locate_DevNode_ExA
@ stdcall CM_Locate_DevNode_ExW(ptr wstr long long) setupapi.CM_Locate_DevNode_ExW
@ stub CM_Merge_Range_List # setupapi.CM_Merge_Range_List
@ stdcall CM_Modify_Res_Des(ptr ptr long ptr long long) setupapi.CM_Modify_Res_Des
@ stdcall CM_Modify_Res_Des_Ex(ptr ptr long ptr long long long) setupapi.CM_Modify_Res_Des_Ex
@ stdcall CM_Move_DevNode(long long long) setupapi.CM_Move_DevNode
@ stdcall CM_Move_DevNode_Ex(long long long long) setupapi.CM_Move_DevNode_Ex
@ stub CM_Next_Range # setupapi.CM_Next_Range
@ stdcall CM_Open_Class_KeyA(ptr str long long ptr long) setupapi.CM_Open_Class_KeyA
@ stdcall CM_Open_Class_KeyW(ptr wstr long long ptr long) setupapi.CM_Open_Class_KeyW
@ stdcall CM_Open_Class_Key_ExA(ptr str long long ptr long long) setupapi.CM_Open_Class_Key_ExA
@ stdcall CM_Open_Class_Key_ExW(ptr wstr long long ptr long long) setupapi.CM_Open_Class_Key_ExW
@ stdcall CM_Open_DevNode_Key(ptr long long long ptr long) setupapi.CM_Open_DevNode_Key
@ stdcall CM_Open_DevNode_Key_Ex(ptr long long long ptr long long) setupapi.CM_Open_DevNode_Key_Ex
@ stdcall CM_Query_And_Remove_SubTreeA(long ptr str long long) setupapi.CM_Query_And_Remove_SubTreeA
@ stdcall CM_Query_And_Remove_SubTreeW(long ptr wstr long long) setupapi.CM_Query_And_Remove_SubTreeW
@ stdcall CM_Query_And_Remove_SubTree_ExA(long ptr str long long long) setupapi.CM_Query_And_Remove_SubTree_ExA
@ stdcall CM_Query_And_Remove_SubTree_ExW(long ptr wstr long long long) setupapi.CM_Query_And_Remove_SubTree_ExW
@ stdcall CM_Query_Arbitrator_Free_Data(ptr long long long long) setupapi.CM_Query_Arbitrator_Free_Data
@ stdcall CM_Query_Arbitrator_Free_Data_Ex(ptr long long long long ptr) setupapi.CM_Query_Arbitrator_Free_Data_Ex
@ stdcall CM_Query_Arbitrator_Free_Size(ptr long long long) setupapi.CM_Query_Arbitrator_Free_Size
@ stdcall CM_Query_Arbitrator_Free_Size_Ex(ptr long long long ptr) setupapi.CM_Query_Arbitrator_Free_Size_Ex
@ stdcall CM_Query_Remove_SubTree(long long) setupapi.CM_Query_Remove_SubTree
@ stdcall CM_Query_Remove_SubTree_Ex(long long long) setupapi.CM_Query_Remove_SubTree_Ex
@ stub CM_Query_Resource_Conflict_List # setupapi.CM_Query_Resource_Conflict_List
@ stdcall CM_Reenumerate_DevNode(long long) setupapi.CM_Reenumerate_DevNode
@ stdcall CM_Reenumerate_DevNode_Ex(long long long) setuapi.CM_Reenumerate_DevNode_Ex
@ stdcall CM_Register_Device_Driver(long long) setupapi.CM_Register_Device_Driver
@ stdcall CM_Register_Device_Driver_Ex(long long ptr) setupapi.CM_Register_Device_Driver_Ex
@ stdcall CM_Register_Device_InterfaceA(long ptr str str ptr long) setupapi.CM_Register_Device_InterfaceA
@ stdcall CM_Register_Device_InterfaceW(long ptr wstr wstr ptr long) setupapi.CM_Register_Device_InterfaceW
@ stdcall CM_Register_Device_Interface_ExA(long ptr str str ptr long long) setupapi.CM_Register_Device_Interface_ExA
@ stdcall CM_Register_Device_Interface_ExW(long ptr wstr wstr ptr long long) setupapi.CM_Register_Device_Interface_ExW
@ stdcall CM_Remove_SubTree(long long) setupapi.CM_Remove_SubTree
@ stdcall CM_Remove_SubTree_Ex(long long long) setupapi.CM_Remove_SubTree_Ex
@ stdcall CM_Request_Eject_PC() setupapi.CM_Request_Eject_PC
@ stdcall CM_Request_Eject_PC_Ex(long) setupapi.CM_Request_Eject_PC_Ex
@ stdcall CM_Run_Detection(long) setupapi.CM_Run_Detection
@ stdcall CM_Run_Detection_Ex(long long) setupapi.CM_Run_Detection_Ex
@ stub CM_Set_Class_Registry_PropertyA # setupapi.CM_Set_Class_Registry_PropertyA
@ stub CM_Set_Class_Registry_PropertyW # setupapi.CM_Set_Class_Registry_PropertyW
@ stdcall CM_Set_DevNode_Problem(long long long) setupapi.CM_Set_DevNode_Problem
@ stdcall CM_Set_DevNode_Problem_Ex(long long long long) setupapi.CM_Set_DevNode_Problem_Ex
@ stdcall CM_Set_DevNode_Registry_PropertyA(long long ptr long long) setupapi.CM_Set_DevNode_Registry_PropertyA
@ stdcall CM_Set_DevNode_Registry_PropertyW(long long ptr long long) setupapi.CM_Set_DevNode_Registry_PropertyW
@ stdcall CM_Set_DevNode_Registry_Property_ExA(long long ptr long long long) setupapi.CM_Set_DevNode_Registry_Property_ExA
@ stdcall CM_Set_DevNode_Registry_Property_ExW(long long ptr long long long) setupapi.CM_Set_DevNode_Registry_Property_ExW
@ stdcall CM_Set_HW_Prof(long long) setupapi.CM_Set_HW_Prof
@ stdcall CM_Set_HW_Prof_Ex(long long long) setupapi.CM_Set_HW_Prof_Ex
@ stdcall CM_Set_HW_Prof_FlagsA(str long long long) setupapi.CM_Set_HW_Prof_FlagsA
@ stdcall CM_Set_HW_Prof_FlagsW(wstr long long long) setupapi.CM_Set_HW_Prof_FlagsW
@ stdcall CM_Set_HW_Prof_Flags_ExA(str long long long long) setupapi.CM_Set_HW_Prof_Flags_ExA
@ stdcall CM_Set_HW_Prof_Flags_ExW(wstr long long long long) setupapi.CM_Set_HW_Prof_Flags_ExW
@ stdcall CM_Setup_DevNode(long long) setupapi.CM_Setup_DevNode
@ stdcall CM_Setup_DevNode_Ex(long long long) setupapi.CM_Setup_DevNode_Ex
@ stub CM_Test_Range_Available # setupapi.CM_Test_Range_Available
@ stdcall CM_Uninstall_DevNode(long long) setupapi.CM_Uninstall_DevNode
@ stdcall CM_Uninstall_DevNode_Ex(long long long) setupapi.CM_Uninstall_DevNode_Ex
@ stdcall CM_Unregister_Device_InterfaceA(str long) setupapi.CM_Unregister_Device_InterfaceA
@ stdcall CM_Unregister_Device_InterfaceW(wstr long) setupapi.CM_Unregister_Device_InterfaceW
@ stdcall CM_Unregister_Device_Interface_ExA(str long long) setupapi.CM_Unregister_Device_Interface_ExA
@ stdcall CM_Unregister_Device_Interface_ExW(wstr long long) setupapi.CM_Unregister_Device_Interface_ExW

# Deprecated functions, they are not present in Win2003 SP1
@ stub CM_Remove_Unmarked_Children # setupapi.CM_Remove_Unmarked_Children
@ stub CM_Remove_Unmarked_Children_Ex # setupapi.CM_Remove_Unmarked_Children_Ex
@ stub CM_Reset_Children_Marks # setupapi.CM_Reset_Children_Marks
@ stub CM_Reset_Children_Marks_Ex # setupapi.CM_Reset_Children_Marks_Ex
