
#define CREATE_EVENT_LISTENER(class_name)\
class class_name : public IGameEventListener\
{\
public:\
	~class_name() { Source::m_pGameEvents->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* game_event);\
};\

namespace game_events //fuck namespaces, fuck ur style bolbi
{
	void init();

	//CREATE_EVENT_LISTENER(ItemPurchaseListener);
	CREATE_EVENT_LISTENER(PlayerHurtListener);
	CREATE_EVENT_LISTENER(BulletImpactListener);
	CREATE_EVENT_LISTENER(PlayerDeathListener);
	CREATE_EVENT_LISTENER(RoundStartListener);
	CREATE_EVENT_LISTENER(RoundEndListener);
};