*** 한글로 대답하기 ***

아무리 생각해도 구조가 너무 더러워. 네이밍이 일단 너무 더럽고 여러 객체상호작용하는것도 마음에 안들어.

UHktSimulationStashComponent 이걸 없애고 UHktIntentEventComponent 에서 모두 처리하고 시뮬레이션만 IHktSimulationProvider에게 부탁하는게 좋겠다.
 
UHktIntentEventComponent 
[클라이언트] Server_NotifyIntent(): 서버에게 IntentEvent 보냄
[서버] Server_ReceiveIntent_Implementation():  서버에서만 IntentEvent 저장( PendingIntentEvents ). 이는 동기화하지 않음.
[서버]  Server_NotifyIntentBatch(): AHktGameMode가 호출하며 틱을 돌리면서 스텝 단위로 프레임의 시작을 알린다. 동시에 PendingIntentEvents의 이벤트들을 하나로 묶어서 FHktIntentEventBatch를 만들고 클라에게 보낸다.
[클라이언트 ] Client_NotifyIntentBatch() :  ProcessingIntentEventBatch 저장하고ProcessingIntentEventBatch를 IHktSimulationProvider에게 처리를 위임하여 HktSimulationModel로 반환할 것을 요청한다. 또한 ProcessingIntentEventBatch는 최초의 접근한 클라에게만 동기화되도록 설정한다. 
[클라이언트 ] Server_NotifyCompletedSimulation() :  서버만 HktSimulationModel를 저장. 저장한 Model 데이터는 최초의 접근한 클라에게만 동기화되도록 설정한다.