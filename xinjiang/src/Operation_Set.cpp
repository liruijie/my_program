/*
 * Operation_Set.cpp
 *
 *  Created on: 2016年7月20日
 *      Author: root
 */
#include "include.h"
extern pthread_mutex_t UpdateSignalParameter;
int SetResult;
/************************设置阶段******************************/
int Set_StageParam(xmlNodePtr OperationNode,int ThreadNum)
{
	//char NodeName[20];
	int ObjectNum = 0;
	int PhaseCount=0;
	int CrossID=0;
	struct Stage_Param StageInfo[64];
	struct ITC_CFG_STAGE_DETAIL StageDetail[64];
	char sqlbuf[500];
	ResultSet *Result;
	xmlChar *temp;

	xmlNodePtr Object = OperationNode ->children ->children;          //operation节点->TSCCmdt节点->要操作的对象节点
	xmlNodePtr CurrNode;
	if(Object == NULL)
	{
		return SDE_Failure;
	}
	while(Object != NULL)
	{
		if( strncmp((char *)Object->name,(char *)&StageParam,sizeof(StageParam) ) != 0 )
		{
			return SDE_Failure;
		}
		CurrNode = Object->children;
		if(Get_Node(CurrNode,(char *)"CrossID") != succeed)
		{
			return SDE_Failure;
		}
		//保证一个Object节点下的路口ID是相同的
		if(ObjectNum == 0)
		{
			temp = xmlNodeGetContent(CurrNode);
			StageInfo[ObjectNum].CrossID = atoi((char *) temp);
			xmlFree(temp);
		}
		else
		{
			temp = xmlNodeGetContent(CurrNode);
			CrossID= atoi((char *)temp);
			xmlFree(temp);
			if(StageInfo[0].CrossID != CrossID)
			{
				return SDE_Failure;
			}
			StageInfo[ObjectNum].CrossID = CrossID;
		}

		CurrNode = Object->children;
		if(Get_Node(CurrNode,(char *)"CoordPhaseNo") != succeed)
		{
			return SDE_Failure;
		}
		temp = xmlNodeGetContent(CurrNode);
		StageInfo[ObjectNum].StageNo = atoi((char *)temp);
		xmlFree(temp);

		CurrNode = Object->children;
		if(Get_Node(CurrNode,(char *)"StageName") != succeed)
		{
			return SDE_Failure;
		}
		temp = xmlNodeGetContent(CurrNode);
		strcpy(StageInfo[ObjectNum].StageName,(char *)temp);
		xmlFree(temp);

		CurrNode = Object->children;
		if(Get_Node(CurrNode,(char *)"Attribute") != succeed)
		{
			return SDE_Failure;
		}
		temp = xmlNodeGetContent(CurrNode);
		StageInfo[ObjectNum].Attribute = atoi((char *)temp);
		xmlFree(temp);

		CurrNode = Object->children;
		if(Get_Node(CurrNode,(char *)"Green") != succeed)
		{
			return SDE_Failure;
		}
		temp = xmlNodeGetContent(CurrNode);
		StageInfo[ObjectNum].Green = atoi((char *)temp);
		xmlFree(temp);

		CurrNode = Object->children;
		if(Get_Node(CurrNode,(char *)"PhaseNoList") != succeed)
		{
			return SDE_Failure;
		}
		//放行相位序号列表。包含至少一个放行相位序号<PhaseNo>
		CurrNode = CurrNode ->children;
		if(CurrNode == NULL)
		{
			return SDE_Failure;
		}
		PhaseCount = 0;
		while(CurrNode != NULL)
		{
			temp = xmlNodeGetContent(CurrNode);
			StageInfo[ObjectNum].PhaseNoList[PhaseCount] = atoi((char *)temp);
			xmlFree(temp);
			PhaseCount++;
			CurrNode = CurrNode->next;
		}
		if(PhaseCount == 0)
		{
			return SDE_Failure;
		}
		ObjectNum++;
		Object = Object ->next;
	}
	if(ObjectNum == 0)
	{
		return SDE_Failure;
	}

	/*
	 * 所有参数获取成功  更新数据库
	 */

	int StageDetailCount=0;
	try
	{
		memset(sqlbuf,'\0',sizeof(sqlbuf));
		sprintf(sqlbuf,"select t.stage_id,t.stage_image ITC_CFG_STAGE_DETAIL t where t.signal_id=%d",StageInfo[0].CrossID);
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		Result = TcpThread[ThreadNum].stmt->executeQuery();

		while(Result->next() != 0)                //cun zai
		{

			StageDetail[StageDetailCount].stage_id = Result->getInt(1);
			strcpy(StageDetail[StageDetailCount].stage_image , Result->getString(2).c_str());
			StageDetailCount++;
		}
		TcpThread[ThreadNum].stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		TcpThread[ThreadNum].stmt->closeResultSet(Result);
		cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return SDE_Failure;
	}

	try
	{
		memset(sqlbuf,'\0',sizeof(sqlbuf));
		sprintf(sqlbuf,"delete from ITC_CFG_STAGE_DETAIL t where t.signal_id=%d",StageInfo[0].CrossID);
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		TcpThread[ThreadNum].stmt->executeUpdate();

		memset(sqlbuf,'\0',sizeof(sqlbuf));
		sprintf(sqlbuf,"delete from ITC_CFG_STAGE t where t.signal_id=%d",StageInfo[0].CrossID);
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		TcpThread[ThreadNum].stmt->executeUpdate();
	}
	catch(SQLException &sqlExcp)
	{
		cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return SDE_Failure;
	}
	int i,j;
	try
	{
		TcpThread[ThreadNum].stmt-> setAutoCommit(false);
		memset(sqlbuf,'\0',sizeof(sqlbuf));
		sprintf(sqlbuf,"insert into ITC_CFG_STAGE t(t.signal_id,t.stage_id,t.stage_phase,t.shield_occ,t.relieve_occ,t.stat_time,t.auto_occ_control,t.stage_step,t.auto_control_time) values(:x1,:x2,:x3,:x4,:x5,:x6,:x7,:x8,:x9)");
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		TcpThread[ThreadNum].stmt->setMaxIterations(200);

		TcpThread[ThreadNum].stmt->setMaxParamSize(1,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(2,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(3,300);
		TcpThread[ThreadNum].stmt->setMaxParamSize(4,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(5,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(6,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(7,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(8,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(9,sizeof(int));
		char temp_buf[300];
		char num[6];
		for(i = 0; i < ObjectNum;i++)
		{
			if(i != 0 )
			{
				TcpThread[ThreadNum].stmt->addIteration();
			}
			TcpThread[ThreadNum].stmt->setNumber(1,StageInfo[i].CrossID);
			TcpThread[ThreadNum].stmt->setNumber(2,StageInfo[i].StageNo);
			j=0;
			memset(temp_buf,'\0',sizeof(temp_buf));
			while(StageInfo[i].PhaseNoList[j] != '\0')
			{
				sprintf(num,"%d,",StageInfo[i].PhaseNoList[j]);
				strcat(temp_buf,num);
				j++;
			}
			temp_buf[strlen(temp_buf)] = '\0';
			TcpThread[ThreadNum].stmt->setString(3,temp_buf);
			TcpThread[ThreadNum].stmt->setNumber(4,0);
			TcpThread[ThreadNum].stmt->setNumber(5,0);
			TcpThread[ThreadNum].stmt->setNumber(6,0);
			TcpThread[ThreadNum].stmt->setNumber(7,0);
			TcpThread[ThreadNum].stmt->setNumber(8,0);
			TcpThread[ThreadNum].stmt->setNumber(9,0);
		}
		TcpThread[ThreadNum].stmt->executeUpdate();
		TcpThread[ThreadNum].conn->commit();
		TcpThread[ThreadNum].stmt-> setAutoCommit(true);
	}
	catch(SQLException &sqlExcp)
	{
		cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		TcpThread[ThreadNum].stmt-> setAutoCommit(true);
		return SDE_Failure;
	}

	try
	{
		TcpThread[ThreadNum].stmt-> setAutoCommit(false);
		memset(sqlbuf,'\0',sizeof(sqlbuf));
		sprintf(sqlbuf,"insert into ITC_CFG_STAGE_DETAIL t(t.signal_id,t.stage_id,t.stage_image,t.detail) values(:x1,:x2,:x3,:x4)");
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		TcpThread[ThreadNum].stmt->setMaxIterations(200);

		TcpThread[ThreadNum].stmt->setMaxParamSize(1,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(2,sizeof(int));
		TcpThread[ThreadNum].stmt->setMaxParamSize(3,20);
		TcpThread[ThreadNum].stmt->setMaxParamSize(4,300);

		for(i = 0; i < ObjectNum;i++)
		{
			if(i!=0)
			{
				TcpThread[ThreadNum].stmt->addIteration();
			}
			TcpThread[ThreadNum].stmt->setNumber(1,StageInfo[i].CrossID);
			TcpThread[ThreadNum].stmt->setNumber(2,StageInfo[i].StageNo);
			TcpThread[ThreadNum].stmt->setString(3,"");
			for(j=0;j<StageDetailCount;j++)
			{
				if(StageInfo[i].StageNo == StageDetail[j].stage_id)
				{
					TcpThread[ThreadNum].stmt->setString(3,StageDetail[j].stage_image);
					break;
				}
			}
			TcpThread[ThreadNum].stmt->setString(4,StageInfo[i].StageName);

		}
		TcpThread[ThreadNum].stmt->executeUpdate();
		TcpThread[ThreadNum].conn->commit();
		TcpThread[ThreadNum].stmt-> setAutoCommit(true);
	}
	catch(SQLException &sqlExcp)
	{
		cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		TcpThread[ThreadNum].stmt-> setAutoCommit(true);
		return SDE_Failure;
	}
	try
	{
		//memset(sqlbuf,'\0',sizeof(sqlbuf));
		//sprintf(sqlbuf,"update ITC_CFG_PLAN_CHAIN t set t.stage_time=%d where t.signal_id=%s and t.stage_id=%d");
		for(i = 0; i < ObjectNum;i++)
		{
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"update ITC_CFG_PLAN_CHAIN t set t.stage_time=%d where t.signal_id=%d and t.stage_id=%d",StageInfo[i].Green,StageInfo[i].CrossID,StageInfo[i].StageNo);
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			TcpThread[ThreadNum].stmt->executeUpdate();
		}
	}
	catch(SQLException &sqlExcp)
	{
		cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
		cout <<  " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return SDE_Failure;
	}

	//发送设置报文 接收设置回复后的信号
	pthread_mutex_lock(&UpdateSignalParameter);
	SetResult = 0;
	if(signal(SIGUSR1,signalfunc) == SIG_ERR)
	{
		printf("get sigusr1 error\n");
	}
	if(signal(SIGUSR2,signalfunc) == SIG_ERR)
	{
		printf("get sigusr2 error\n");
	}
	UpdateParam(StageInfo[i].CrossID);

	for(i=0;i<30;i++)
	{
		sleep(1);
	}
	pthread_mutex_unlock(&UpdateSignalParameter);
	if(SetResult != 1)
	{
		return fail;
	}

	return succeed;
}
/**************************设置方案**********************************/
int Set_PlanParam(xmlNodePtr OperationNode,int ThreadNum)
{

		int ObjectNum = 0;
		int PhaseCount=0;
		//char NodeName[20];
		struct Plan_Param PlanInfo[128];
		struct Itc_Cfg_Plan_Detail PlanDetail[128];
		struct ITC_CFG_PLAN_CHAIN PlanChain[128];
		int PlanDetailCount=0;
		int PlanChainCount=0;
		ResultSet *Result;
		char sqlbuf[500];
		xmlNodePtr Object = OperationNode ->children ->children;          //operation节点->TSCCmdt节点->要操作的对象节点
		xmlNodePtr CurrNode;
		xmlChar *tempbuf;
		if(Object == NULL)
		{
			return SDE_Failure;
		}
		while(Object != NULL)
		{
			if( strncmp((char *)Object->name,(char *)&PlanParam,sizeof(PlanParam) ) != 0 )
			{
				return SDE_Failure;
			}
			CurrNode = Object->children;

			if(Get_Node(CurrNode,(char *)"CrossID" )!= succeed)
			{
				return SDE_Failure;
			}
			tempbuf = xmlNodeGetContent(CurrNode);
			PlanInfo[ObjectNum].CrossID = atoi((char *)tempbuf);
			xmlFree(tempbuf);

			CurrNode = Object->children;
			if(Get_Node(CurrNode,(char *)"PlanNo") != succeed)
			{
				return SDE_Failure;
			}
			tempbuf = xmlNodeGetContent(CurrNode);
			PlanInfo[ObjectNum].PlanNo = atoi((char *)tempbuf);
			xmlFree(tempbuf);

			CurrNode = Object->children;
			if(Get_Node(CurrNode,(char *)"CycleLen") != succeed)
			{
				return SDE_Failure;
			}
			tempbuf = xmlNodeGetContent(CurrNode);
			PlanInfo[ObjectNum].CycleLen = atoi((char *)tempbuf);
			xmlFree(tempbuf);

			CurrNode = Object->children;
			if(Get_Node(CurrNode,(char *)"CoordPhaseNo") != succeed)
			{
				return SDE_Failure;
			}
			tempbuf = xmlNodeGetContent(CurrNode);
			PlanInfo[ObjectNum].CoordPhaseNo = atoi((char *)tempbuf);
			xmlFree(tempbuf);

			CurrNode = Object->children;
			if(Get_Node(CurrNode,(char *)"OffSet") != succeed)
			{
				return SDE_Failure;
			}
			tempbuf = xmlNodeGetContent(CurrNode);
			PlanInfo[ObjectNum].OffSet = atoi((char *)tempbuf);
			xmlFree(tempbuf);


			CurrNode = Object->children;
			if(Get_Node(CurrNode,(char *)"StageNoList") != succeed)
			{
				return SDE_Failure;
			}
			//放行阶段序号列表。包含至少一个阶段序号<StageNo>
			CurrNode = CurrNode ->children;
			if(CurrNode == NULL)
			{
				return SDE_Failure;
			}
			while(CurrNode != NULL)
			{
				tempbuf = xmlNodeGetContent(CurrNode);
				PlanInfo[ObjectNum].StageNoList[PhaseCount] = atoi((char *)tempbuf);
				xmlFree(tempbuf);
				PhaseCount++;
				CurrNode = CurrNode->next;
			}
			if(PhaseCount == 0)
			{
				return SDE_Failure;
			}
			ObjectNum++;
			Object = Object ->next;
		}
		if(ObjectNum == 0)
		{
			return SDE_Failure;
		}

		/* 所有参数获取成功  更新数据库 */
		try
		{
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"select t.plan_id,t.detail from Itc_Cfg_Plan_Detail t where t.signal_id=%d",PlanInfo[0].CrossID);
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			Result = TcpThread[ThreadNum].stmt->executeQuery();

			while(Result->next() != 0)                //cun zai
			{
				PlanDetail[PlanDetailCount].PlanID = Result->getInt(1);
				strcpy(PlanDetail[PlanDetailCount].Detail , Result->getString(2).c_str());
				PlanDetailCount++;
			}
			TcpThread[ThreadNum].stmt->closeResultSet(Result);
		}
		catch(SQLException &sqlExcp)
		{
			TcpThread[ThreadNum].stmt->closeResultSet(Result);
			cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
			cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
			cout << sqlExcp.getMessage() << endl; //获得异常信息
			return SDE_Failure;
		}
		try
		{
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"select t.stage_id,t.stage_time,t.stage_type,t.green_time,t.green_flash_time,t.green_flash_qtime,t.red_time from ITC_CFG_PLAN_CHAIN t where t.signal_id = %d",PlanInfo[0].CrossID);
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			Result = TcpThread[ThreadNum].stmt->executeQuery();

			while(Result->next() != 0)                //cun zai
			{
				PlanChain[PlanChainCount].stage_id =Result->getInt(1);
				PlanChain[PlanChainCount].stage_time =Result->getInt(2);
				PlanChain[PlanChainCount].stage_type= Result->getInt(3);
				PlanChain[PlanChainCount].green_time = Result->getFloat(4);
				PlanChain[PlanChainCount].green_flash_time = Result->getFloat(5);
				PlanChain[PlanChainCount].green_flash_qtime = Result->getFloat(6);
				PlanChain[PlanChainCount].red_time = Result->getFloat(7);
				PlanChainCount++;
			}
			TcpThread[ThreadNum].stmt->closeResultSet(Result);
		}
		catch(SQLException &sqlExcp)
		{
			TcpThread[ThreadNum].stmt->closeResultSet(Result);
			cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
			cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
			cout << sqlExcp.getMessage() << endl; //获得异常信息
			return SDE_Failure;
		}

		try
		{
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"delete from Itc_Cfg_Plan_Detail t where t.signal_id=%d and t.plan_id<129",PlanInfo[0].CrossID);
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			Result = TcpThread[ThreadNum].stmt->executeQuery();
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"delete from ITC_CFG_PLAN_CHAIN t where t.signal_id=%d and t.plan_id<129",PlanInfo[0].CrossID);
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			Result = TcpThread[ThreadNum].stmt->executeQuery();
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"delete from ITC_CFG_PLAN t where t.signal_id=%d and t.plan_id<129",PlanInfo[0].CrossID);
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			Result = TcpThread[ThreadNum].stmt->executeQuery();
		}
		catch(SQLException &sqlExcp)
		{
			TcpThread[ThreadNum].stmt->closeResultSet(Result);
			cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
			cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
			cout << sqlExcp.getMessage() << endl; //获得异常信息
			return SDE_Failure;
		}
		int i,j;
		try
		{
			TcpThread[ThreadNum].stmt-> setAutoCommit(false);
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"insert into ITC_CFG_PLAN t(t.signal_id,t.plan_id,t.list_unit_id,t.plan_week,t.plan_record,t.phase_diff_time) values(:x1,:x2,:x3,:x4,:x5,:x6)");
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);

			TcpThread[ThreadNum].stmt->setMaxIterations(5000);
			TcpThread[ThreadNum].stmt->setMaxParamSize(1,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(2,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(3,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(4,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(5,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(6,sizeof(int));
			for(i = 0; i < ObjectNum;i++)
			{
				if(i!=0)
				{
					TcpThread[ThreadNum].stmt->addIteration();
				}
				TcpThread[ThreadNum].stmt->setNumber(1,PlanInfo[i].CrossID);
				TcpThread[ThreadNum].stmt->setNumber(2,PlanInfo[i].PlanNo);
				TcpThread[ThreadNum].stmt->setNumber(3,1);
				TcpThread[ThreadNum].stmt->setNumber(4,PlanInfo[i].CycleLen);
				TcpThread[ThreadNum].stmt->setNumber(5,PlanInfo[i].CoordPhaseNo);
				TcpThread[ThreadNum].stmt->setNumber(6,PlanInfo[i].OffSet);
			}
			TcpThread[ThreadNum].stmt->executeUpdate();
			TcpThread[ThreadNum].conn->commit();
			TcpThread[ThreadNum].stmt-> setAutoCommit(true);
		}
		catch(SQLException &sqlExcp)
		{
			cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
			cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
			cout << sqlExcp.getMessage() << endl; //获得异常信息
			TcpThread[ThreadNum].stmt-> setAutoCommit(true);
			return SDE_Failure;
		}
		try
		{
			TcpThread[ThreadNum].stmt-> setAutoCommit(false);
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"insert into Itc_Cfg_Plan_Detail t(t.signal_id,t.plan_id,t.detail) values(:x1,:x2,:x3)");
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			TcpThread[ThreadNum].stmt->setMaxIterations(5000);

			TcpThread[ThreadNum].stmt->setMaxParamSize(1,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(2,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(3,100);
			for(i = 0; i < ObjectNum;i++)
			{
				if(i!=0)
				{
					TcpThread[ThreadNum].stmt->addIteration();
				}
				TcpThread[ThreadNum].stmt->setNumber(1,PlanInfo[i].CrossID);
				TcpThread[ThreadNum].stmt->setNumber(2,PlanInfo[i].PlanNo);
				TcpThread[ThreadNum].stmt->setString(3,"");
				for(j = 0;j<PlanDetailCount;j++)
				{
					if(PlanInfo[i].PlanNo == PlanDetail[j].PlanID)
					{
						TcpThread[ThreadNum].stmt->setString(3,PlanDetail[j].Detail);
						break;
					}
				}
			}
			TcpThread[ThreadNum].stmt->executeUpdate();
			TcpThread[ThreadNum].conn->commit();
			TcpThread[ThreadNum].stmt-> setAutoCommit(true);
		}
		catch(SQLException &sqlExcp)
		{
			cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
			cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
			cout << sqlExcp.getMessage() << endl; //获得异常信息
			TcpThread[ThreadNum].stmt-> setAutoCommit(true);
			return SDE_Failure;
		}
		try
		{
			TcpThread[ThreadNum].stmt-> setAutoCommit(false);
			memset(sqlbuf,'\0',sizeof(sqlbuf));
			sprintf(sqlbuf,"insert into ITC_CFG_PLAN_CHAIN t(t.signal_id,t.plan_id,t.stage_order,t.stage_id,t.stage_time,t.stage_type,t.green_time,t.green_flash_time,t.green_flash_qtime,t.red_time) values(:x1,:x2,:x3,:x4,:x5,:x6,:x7,:x8,:x9,:x10)");
			TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
			TcpThread[ThreadNum].stmt->setMaxIterations(5000);

			TcpThread[ThreadNum].stmt->setMaxParamSize(1,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(2,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(3,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(4,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(5,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(6,sizeof(int));
			TcpThread[ThreadNum].stmt->setMaxParamSize(7,sizeof(float));
			TcpThread[ThreadNum].stmt->setMaxParamSize(8,sizeof(float));
			TcpThread[ThreadNum].stmt->setMaxParamSize(9,sizeof(float));
			TcpThread[ThreadNum].stmt->setMaxParamSize(10,sizeof(float));
			int k;
			for(i = 0; i < ObjectNum;i++)
			{

				for(j = 0;PlanInfo[i].StageNoList[j] != '\0';j++)
				{
					if(i!=0)
					{
						TcpThread[ThreadNum].stmt->addIteration();
					}
					TcpThread[ThreadNum].stmt->setNumber(1,PlanInfo[i].CrossID);
					TcpThread[ThreadNum].stmt->setNumber(2,PlanInfo[i].PlanNo);
					TcpThread[ThreadNum].stmt->setNumber(3,i);
					TcpThread[ThreadNum].stmt->setNumber(4,PlanInfo[i].StageNoList[j]);
					TcpThread[ThreadNum].stmt->setNumber(5,0);
					TcpThread[ThreadNum].stmt->setNumber(6,0);
					TcpThread[ThreadNum].stmt->setNumber(7,0);
					TcpThread[ThreadNum].stmt->setNumber(8,0);
					TcpThread[ThreadNum].stmt->setNumber(9,0);
					TcpThread[ThreadNum].stmt->setNumber(10,0);
					for(k=0;k < PlanChainCount; k++)
					{
						if(PlanInfo[i].StageNoList[j] == PlanChain[k].stage_id)
						{
							TcpThread[ThreadNum].stmt->setNumber(5,PlanChain[k].stage_time);
							TcpThread[ThreadNum].stmt->setNumber(6,PlanChain[k].stage_type);
							TcpThread[ThreadNum].stmt->setNumber(7,PlanChain[k].green_time);
							TcpThread[ThreadNum].stmt->setNumber(8,PlanChain[k].green_flash_time);
							TcpThread[ThreadNum].stmt->setNumber(9,PlanChain[k].green_flash_qtime);
							TcpThread[ThreadNum].stmt->setNumber(10,PlanChain[k].red_time);
							break;
						}
					}
					/******/
				}
			}
			TcpThread[ThreadNum].stmt->executeUpdate();
			TcpThread[ThreadNum].conn->commit();
			TcpThread[ThreadNum].stmt-> setAutoCommit(true);
		}
		catch(SQLException &sqlExcp)
		{
			cout  << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<<endl;
			cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
			cout << sqlExcp.getMessage() << endl; //获得异常信息
			TcpThread[ThreadNum].stmt-> setAutoCommit(true);
			return SDE_Failure;
		}
		pthread_mutex_lock(&UpdateSignalParameter);
		SetResult = 0;
		if(signal(SIGUSR1,signalfunc) == SIG_ERR)
		{
			printf("get sigusr1 error\n");
		}
		if(signal(SIGUSR2,signalfunc) == SIG_ERR)
		{
			printf("get sigusr2 error\n");
		}
		UpdateParam(PlanInfo[0].CrossID);

		for(i=0;i<30;i++)
		{
			sleep(1);
		}
		pthread_mutex_unlock(&UpdateSignalParameter);
		if(SetResult != 1)
		{
			return fail;
		}
	return succeed;
}

/*********************设置实时上报内容********************************/
int Set_CrossReportCtrl(xmlNodePtr OperationNode,int ThreadNum)
{
	xmlNodePtr Node = OperationNode ->children->children;
	xmlNodePtr CrossListNode;
	char *Cmd;
	char *Type;
	bool CmdValue;
	int TypeValue=0;   //    1:CrossCycle   2:CrossStage  3:CrossPhaseLampStatus  4:CrossTrafficData    多了可以用define方式
	char *SignalID_buf;
	int SignalID=0;

	if(Node == NULL)
	{
		return SDE_Failure;
	}
	while(1)             //获取 命令   操作对象  和路口列表节点
	{
		if(Node == NULL)
		{
			break;
		}
		else if( strncmp( (char *)Node->name,"Cmd",3) == 0)
		{
			Cmd = (char *)xmlNodeGetContent(Node);
			continue;
		}
		else if( strncmp( (char *)Node->name,"Type",4) == 0)
		{
			Type = (char *)xmlNodeGetContent(Node);
			continue;
		}
		else if( strncmp( (char *)Node->name,"CrossIDList",11) == 0)
		{
			CrossListNode = Node;
			continue;
		}
		else
		{
			Node = Node ->next;
		}
	}
	if( (Cmd == NULL) || (Type == NULL) || (CrossListNode == NULL) )           //判断是否获取成功
	{
		if(Cmd != NULL)
			xmlFree(Cmd);
		if(Type != NULL)
			xmlFree(Type);
		if(CrossListNode != NULL)
			xmlFree(CrossListNode);
		return SDE_Failure;
	}

	if( strncmp( Cmd,"Start",5) == 0)
	{
		CmdValue = true;
	}
	else if( strncmp( Cmd,"Stop",4) == 0)
	{
		CmdValue = false;
	}
	else
	{
		xmlFree(Cmd);
		return SDE_Failure;
	}
	xmlFree(Cmd);

	if( strncmp( Type,"CrossCycle",10) == 0)
	{
		TypeValue = 1;
	}
	else if( strncmp( Type,"CrossStage",10) == 0)
	{
		TypeValue = 2;
	}
	else if( strncmp( Type,"CrossPhaseLampStatus",20) == 0)
	{
		TypeValue = 3;
	}
	else if( strncmp( Type,"CrossTrafficData",16) == 0)
	{
		TypeValue = 4;
	}
	else
	{
		xmlFree(Type);
		return SDE_NotAllow;
	}
	xmlFree(Type);


	int i;
	Node = CrossListNode->children;
	while(1)
	{
		if(Node == NULL)
		{
			return SDE_Failure;
		}
		else if( strncmp( (char *)Node->name,"CrossID",7) == 0)
		{
			SignalID_buf = (char *)xmlNodeGetContent(Node);
			if(SignalID_buf == NULL)
				return SDE_Failure;
			SignalID = atoi(SignalID_buf);
			if(SignalID == 0)
			{
				xmlFree(SignalID_buf);
				SignalID_buf = NULL;
				continue;
			}
			for(i = 0;i < SignalMaxNum;i++)                      //
			{
				if(TcpThread[ThreadNum].ReportInfo[i].CrossID == SignalID)
				{
					switch(TypeValue)
					{
						case 1:
							TcpThread[ThreadNum].ReportInfo[i].CrossCycle_flag = CmdValue;
							break;
						case 2:
							TcpThread[ThreadNum].ReportInfo[i].CrossStage_flag = CmdValue;
							break;
						case 3:
							TcpThread[ThreadNum].ReportInfo[i].CrossPhaseLampStatus_flag = CmdValue;
							break;
						case 4:
							TcpThread[ThreadNum].ReportInfo[i].CrossTrafficData_flag = CmdValue;
							break;
						default:
							break;
					}
				}
			}
			xmlFree(SignalID_buf);
			SignalID_buf = NULL;
		}
		else
		{
			Node = Node ->next;
		}
	}
	return succeed;
}

/***********************锁定交通流向*******************************/
int Set_LockFlowDirection(xmlNodePtr OperationNode,int ThreadNum)
{
	xmlNodePtr Node = OperationNode ->children->children;
	char *Cmd;
	char *tempbuf;
	int CrossID;
	int Direction;
	int ret;
	int i;
	char sqlbuf[200];
	ResultSet *Result;
	int PhaseNum;
	struct tm timeinfo;
	/*xmlNodePtr CrossListNode;

	char *Type;
	bool CmdValue;
	int TypeValue=0;   //    1:CrossCycle   2:CrossStage  3:CrossPhaseLampStatus  4:CrossTrafficData    多了可以用define方式
	char *SignalID_buf;
	int SignalID=0;*/

	if(Node == NULL)
	{
		return SDE_Failure;
	}
	while(1)             //获取 命令   操作对象  和路口列表节点
	{
		if(Node == NULL)
		{
			break;
		}
		if( strncmp( (char *)Node->name,"CrossID",7) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			CrossID = atoi(tempbuf);
			xmlFree(tempbuf);
			for(i = 0;i<SignalMaxNum;i++)
			{
				if(SignalRealData[i].CrossID == CrossID)
				{
					break;
				}
			}
			if( i == SignalMaxNum)
				return SDE_Failure;
			if(SignalRealData[i].ControlFunction != Ctr_Idle)    //如果当前状态是被控制中
				return SDE_Failure;
			continue;
		}
		if( strncmp( (char *)Node->name,"Type",4) == 0)
		{
			//Cmd = (char *)xmlNodeGetContent(Node);
			continue;
		}
		if( strncmp( (char *)Node->name,"Entrance",8) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			Direction = atoi(tempbuf);
			xmlFree(tempbuf);
			ret = GetDirection(Direction);
			if(ret == fail)
			{
				return SDE_Failure;
			}
			SignalRealData[i].LockEnter = ret;
			continue;
		}
		if( strncmp( (char *)Node->name,"Exit",4) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			Direction = atoi(tempbuf);
			xmlFree(tempbuf);
			ret = GetDirection(Direction);
			if(ret == fail)
			{
				return SDE_Failure;
			}
			SignalRealData[i].LockExit = ret;
			continue;
		}
		if( strncmp( (char *)Node->name,"StartTime",9) == 0)
		{
			Cmd = NULL;
			char fmt[] = "%Y-%m-%d %H:%M:%S";
			Cmd = (char *)xmlNodeGetContent(Node);
			if( (Cmd ==NULL) || (Cmd[0] = '0'))         //时间为0  需要立即进行锁定
			{
				if((SignalRealData[i].ControlFunction == Ctr_Idle) && (SignalRealData[i].IsControlled == false) )   //
				{
					SignalRealData[i].LockStartTime = 0;
				}
			}
			else if (strptime(Cmd, fmt, &timeinfo) != NULL)
			{
				SignalRealData[i].LockStartTime = mktime(&timeinfo);
				SignalRealData[i].IsExistLockTask = true;
			}
			xmlFree(Cmd);
			continue;
		}
		if( strncmp( (char *)Node->name,"Duration",8) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			SignalRealData[i].LockDuration =atoi(tempbuf);
			xmlFree(tempbuf);
			continue;
		}
		Node = Node->next;
	}
	sprintf(sqlbuf,"select p.phase_id from UNIT_PHASE_CANALIZATION_CFG p where p.signal_id=%d and instr(','||p.canalization_id||',',(select ','||t.id||',' from UNIT_CANALIZATION_CFG t where t.direction_enter=%d and t.direction_exit=%d))>0",CrossID,SignalRealData[i].LockEnter,SignalRealData[i].LockExit);
	try
	{
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		Result = TcpThread[ThreadNum].stmt->executeQuery();
		ret = 0;
		while(Result->next() != 0)                //cun zai
		{
			PhaseNum = Result->getInt(1);
			ret++;
			break;
		}
		TcpThread[ThreadNum].stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return SDE_Failure;
	}
	if(ret == 0)
	{
		return SDE_Failure;
	}
	sprintf(sqlbuf,"select t.stage_id from ITC_CFG_STAGE t where t.signal_id=%d and instr(','||t.stage_phase||',',',%d,')>0 order by length(t.stage_phase)",CrossID,PhaseNum);
	try
	{
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		Result = TcpThread[ThreadNum].stmt->executeQuery();
		ret = 0;
		while(Result->next() != 0)                //cun zai
		{
			SignalRealData[i].LockStageNum = Result->getInt(1);
			ret++;
			break;
		}
		TcpThread[ThreadNum].stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return SDE_Failure;
	}
	return succeed;
}

/************************解锁交通流向*******************************/
int Set_UnLockFlowDirection(xmlNodePtr OperationNode,int ThreadNum)
{
	xmlNodePtr Node = OperationNode ->children->children;
	char *Cmd;
	char *tempbuf;
	int CrossID;
	int Direction;
	int ret;
	int i;
	char sqlbuf[200];
	ResultSet *Result;
	int PhaseNum;
	/*xmlNodePtr CrossListNode;*/
	int LockStageNum;

	if(Node == NULL)
	{
		return SDE_Failure;
	}
	while(1)             //获取 命令   操作对象  和路口列表节点
	{
		if(Node == NULL)
		{
			break;
		}
		if( strncmp( (char *)Node->name,"CrossID",3) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			CrossID = atoi(tempbuf);
			xmlFree(tempbuf);
			for(i = 0;i<SignalMaxNum;i++)
			{
				if(SignalRealData[i].CrossID == CrossID)
				{
					break;
				}
			}
			if( i == SignalMaxNum)
				return SDE_Failure;
			if(SignalRealData[i].ControlFunction != Ctr_LockTraffic)          //如果当前状态不是锁定交通流
				return SDE_Failure;
		}
		if( strncmp( (char *)Node->name,"Type",3) == 0)
		{
			//Cmd = (char *)xmlNodeGetContent(Node);
			continue;
		}
		if( strncmp( (char *)Node->name,"Entrance",3) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			Direction = atoi(tempbuf);
			xmlFree(tempbuf);
			ret = GetDirection(Direction);
			if(ret == fail)
			{
				return SDE_Failure;
			}
			SignalRealData[i].LockEnter = ret;
			continue;
		}
		if( strncmp( (char *)Node->name,"Exit",3) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			Direction = atoi(tempbuf);
			xmlFree(tempbuf);
			ret = GetDirection(Direction);
			if(ret == fail)
			{
				return SDE_Failure;
			}
			SignalRealData[i].LockExit = ret;
			continue;
		}
		if( strncmp( (char *)Node->name,"StartTime",3) == 0)
		{
			Cmd = (char *)xmlNodeGetContent(Node);
			xmlFree(Cmd);
			continue;
		}
		if( strncmp( (char *)Node->name,"Duration",3) == 0)
		{
			tempbuf = (char *)xmlNodeGetContent(Node);
			SignalRealData[i].LockDuration =atoi(tempbuf);
			xmlFree(tempbuf);
			continue;
		}
		Node = Node->next;
	}
	sprintf(sqlbuf,"select p.phase_id from UNIT_PHASE_CANALIZATION_CFG p where p.signal_id=%d and instr(','||p.canalization_id||',',(select ','||t.id||',' from UNIT_CANALIZATION_CFG t where t.direction_enter=%d and t.direction_exit=%d))>0",CrossID,SignalRealData[i].LockEnter,SignalRealData[i].LockExit);
	try
	{
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		Result = TcpThread[ThreadNum].stmt->executeQuery();
		ret = 0;
		while(Result->next() != 0)                //cun zai
		{
			PhaseNum = Result->getInt(1);
			ret++;
			break;
		}
		TcpThread[ThreadNum].stmt->closeResultSet(Result);
		if(ret == 0)
		{
			return SDE_Failure;
		}
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return SDE_Failure;
	}

	sprintf(sqlbuf,"select t.stage_id from ITC_CFG_STAGE t where t.signal_id=%d and instr(','||t.stage_phase||',',',%d,')>0 order by length(t.stage_phase)",CrossID,PhaseNum);
	try
	{
		TcpThread[ThreadNum].stmt->setSQL(sqlbuf);
		Result = TcpThread[ThreadNum].stmt->executeQuery();
		ret = 0;
		while(Result->next() != 0)                //cun zai
		{
			LockStageNum = Result->getInt(1);
			ret++;
			break;
		}
		TcpThread[ThreadNum].stmt->closeResultSet(Result);
		if(ret == 0)
		{
			return SDE_Failure;
		}
		if(LockStageNum == SignalRealData[i].LockStageNum)           //如果要解锁的交通流和锁定的一致
		{
			//SignalRealData[i].LockStageNum = tempvalue;
			SignalRealData[i].ControlFunction = Ctr_UnLockTraffic;
		}
		else
		{
			return SDE_Failure;
		}
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return SDE_Failure;
	}
	return succeed;
}

/***********************指定方案*******************************/
int Set_CrossPlan(xmlNodePtr OperationNode,int ThreadNum)
{
	xmlNodePtr ObjectNode = OperationNode ->children->children;
	xmlNodePtr CurrNode = ObjectNode->children;
	char *tempbuf;
	int tempvalue;
	int i;
	int CrossID = 0;

	if(CurrNode == NULL)
	{
		return SDE_Failure;
	}
	if(Get_Node(CurrNode,(char *)"CrossID") == fail)
		return SDE_Failure;

	tempbuf = (char *)xmlNodeGetContent(CurrNode);
	CrossID =  atoi( tempbuf );
	xmlFree(tempbuf);
	for(i = 0;i<SignalMaxNum;i++)
	{
		if(SignalRealData[i].CrossID == CrossID)
		{
			break;
		}
	}
	if( i == SignalMaxNum)
		return SDE_Failure;


	CurrNode = ObjectNode->children;
	if(Get_Node(CurrNode,(char *)"PlanNo") == fail)
		return SDE_Failure;
	tempbuf = (char *)xmlNodeGetContent(CurrNode);
	tempvalue = atoi( tempbuf );
	xmlFree(tempbuf);
	//当前状态是按指定方案运行，如果报文中的阶段号0就是取消该方案
	if( (tempvalue == 0) && (SignalRealData[i].ControlFunction == Ctr_CrossPlan) && (SignalRealData[i].IsControlled == true))
	{
		SignalRealData[i].ControlFunction = Cancel_CrossPlan;
	}
	//如果当前控制状态是空闲
	else if( (tempvalue>0) && (SignalRealData[i].ControlFunction == Ctr_Idle) && (SignalRealData[i].IsControlled == false))      //
	{
		SignalRealData[i].CtrCrossPlan = tempvalue;
		SignalRealData[i].ControlFunction = Ctr_CrossPlan;
	}
	else
	{
		return SDE_Failure;
	}

	return succeed;
}

/***********************指定控制方式*******************************/
int Set_CrossControlMode(xmlNodePtr OperationNode,int ThreadNum)
{
	xmlNodePtr ObjectNode = OperationNode ->children->children;
	xmlNodePtr CurrNode = ObjectNode ->children;
	int i;
	char *tempbuf;
	int tempvalue;
	int CrossID;

	if(Get_Node(CurrNode,(char *)"CrossID") == fail)
		return SDE_Failure;
	tempbuf = (char *)xmlNodeGetContent(CurrNode);
	if(tempbuf == NULL)
		return SDE_Failure;
	CrossID =  atoi( tempbuf );
	xmlFree(tempbuf);
	for(i = 0;i<SignalMaxNum;i++)
	{
		if(SignalRealData[i].CrossID == CrossID)
		{
			break;
		}
	}
	if( i == SignalMaxNum)
		return SDE_Failure;
	if(SignalRealData[i].ControlFunction != Ctr_Idle)          //如果当前状态是被控制中
		return SDE_Failure;
	CurrNode = ObjectNode ->children;
	if(Get_Node(CurrNode,(char *)"Value") == fail)
		return SDE_Failure;
	tempbuf = (char *)xmlNodeGetContent(CurrNode);
	tempvalue = atoi( tempbuf );
	xmlFree(tempbuf);
	//当前状态是按指定模式运行，如果报文中的阶段号0就是取消该运行模式
	if( (tempvalue == 0) && (SignalRealData[i].ControlFunction == Ctr_ControlMode) && (SignalRealData[i].IsControlled == true) )
	{
		SignalRealData[i].ControlFunction = Cancel_ControlMode;
	}
	//如果当前控制状态是空闲
	else if( (tempvalue>0) && (SignalRealData[i].ControlFunction == Ctr_Idle) )      //
	{
		switch(tempvalue)
		{
		/*		11 特殊控制-关灯		12 特殊控制-全红		13 特殊控制-全部黄闪		21 单点多时段定时控制		22 单点感应控制
				23 单点自适应控制		31 线协调控制			41 区域协调控制			51 干预控制-手动控制		52 干预控制-锁定阶段控制
				53 干预控制-指定方案 */
			case 11:
				break;
			case 12:
				break;
			case 13:
				break;
			case 21:
				break;
			case 22:
				break;
			case 23:
				break;
			case 31:
				break;
			case 41:
				break;
			case 51:
				break;
			case 52:
				break;
			case 53:
				break;
			default:
				break;
		}
		SignalRealData[i].CrossContolMode = tempvalue;
		SignalRealData[i].ControlFunction = Ctr_ControlMode;
	}
	else
	{
		return SDE_Failure;
	}

	return succeed;
}

/***********************指定阶段*******************************/
int Set_CrossStage(xmlNodePtr OperationNode,int ThreadNum)
{
		xmlNodePtr ObjectNode = OperationNode ->children->children;
		xmlNodePtr CurrNode = ObjectNode ->children;
		int i;
		char *tempbuf;
		int tempvalue;
		int CrossID;

		if(Get_Node(CurrNode,(char *)"CrossID") == fail)
			return SDE_Failure;
		tempbuf = (char *)xmlNodeGetContent(CurrNode);
		if(tempbuf == NULL)
			return SDE_Failure;
		CrossID =  atoi( tempbuf );
		xmlFree(tempbuf);
		for(i = 0;i<SignalMaxNum;i++)
		{
			if(SignalRealData[i].CrossID == CrossID)
			{
				break;
			}
		}
		if( i == SignalMaxNum)
			return SDE_Failure;

		CurrNode = ObjectNode ->children;
		if(Get_Node(CurrNode,(char *)"CurStageNo") == fail)
			return SDE_Failure;
		tempbuf = (char *)xmlNodeGetContent(CurrNode);

		tempvalue = atoi( tempbuf );
		xmlFree(tempbuf);
		//当前状态是锁定阶段，如果报文中的阶段号0就是取消锁定
		if( (tempvalue == 0) && (SignalRealData[i].ControlFunction == Ctr_CrossStage) && (SignalRealData[i].IsControlled == true))
		{
			SignalRealData[i].ControlFunction = Cancel_CrossStage;
		}
		//如果当前控制状态是空闲，阶段号是有效值，修改状态，准备锁定
		else if( (tempvalue>0) && (SignalRealData[i].ControlFunction == Ctr_Idle) )      //
		{
			SignalRealData[i].CtrCrossStage = tempvalue;
			SignalRealData[i].ControlFunction = Ctr_CrossStage;
		}
		else
		{
			return SDE_Failure;
		}

		CurrNode = ObjectNode ->children;
		if(Get_Node(CurrNode,(char *)"CurStageLen") == fail)
			return SDE_Failure;
		tempbuf = (char *)xmlNodeGetContent(CurrNode);
		SignalRealData[i].CtrStageLen= atoi( tempbuf );
		xmlFree(tempbuf);
		return succeed;
}

int GetDirection(int Direction)
{
	switch(Direction)
	{
		case 0:
				return 4;
		case 1:
				return 8;
		case 2:
				return 1;
		case 3:
				return 5;
		case 4:
				return 3;
		case 5:
				return 7;
		case 6:
				return 2;
		case 7:
				return 6;
		default:
			return SDE_Failure;
	}
}

void signalfunc(int sig)
{
	//int SetResult;
	if(sig == SIGUSR1)
	{
		SetResult = 1;
	}
	else if(sig == SIGUSR2)
	{
		SetResult = 2;
	}
	else
	{
		printf("can not get signal\n");
	}

}
