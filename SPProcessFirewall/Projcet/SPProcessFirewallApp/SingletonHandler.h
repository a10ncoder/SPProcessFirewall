#pragma once

template <typename T>
class CSingletonHandler : public T
{
public:
	//在第一次实例化的时候使用DoubleCheckNull方式检测并发性
	static T* Instance()
	{
		if (m_pInstance == NULL)
		{
			if (m_pInstance == NULL)
            {
                m_pInstance = new T;
            }
		}
		return m_pInstance;
	}
	static void Release()
	{
		if (m_pInstance != NULL)
		{
			if (m_pInstance != NULL)
			{
                delete m_pInstance;
                m_pInstance = NULL;
			}
		}
	}
private:
	CSingletonHandler();
	~CSingletonHandler();
	
	CSingletonHandler(const CSingletonHandler& singletonhandler);
	CSingletonHandler& operator=(const CSingletonHandler& singletonhandler);

	static T* m_pInstance;
};

template <typename T>
T* CSingletonHandler<T>::m_pInstance = NULL;