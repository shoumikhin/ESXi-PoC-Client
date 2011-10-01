#include <iostream>
#include "soapVimBindingProxy.h"
#include "VimBinding.nsmap"
#include "soapStub.h"
#include "soapH.h"
#include <time.h>
#include <iostream>

using namespace std;
void sigpipe_handle(int x) { cout << "sigpipe " << x << endl; }

static ns1__ManagedObjectReference*
getVMManagedObjectRef(VimBinding* vim, ns1__ServiceContent* serviceContent,string vm_name)
{
	ns1__TraversalSpec dataCenterVMTraversalSpec, folderTraversalSpec;
	ns1__SelectionSpec dataCenterVMTraversalSelectionSpec, folderTraversalSelectionSpec;
	ns1__PropertySpec propertySpec;
	ns1__ObjectSpec objectSpec;
	ns1__PropertyFilterSpec propertyFilterSpec;
	ns1__DynamicProperty dynamicProperty;
	ns1__RetrievePropertiesRequestType retrievePropertiesReq;
	_ns1__RetrievePropertiesResponse retrievePropertiesRes;
	ns1__ManagedObjectReference* vm_mor = NULL;

	bool xsd_true = 1;
	bool xsd_false = 0;

	folderTraversalSelectionSpec.name = new string("FolderTraversalSpec");
	dataCenterVMTraversalSelectionSpec.name = new string("DataCenterVMTraversalSpec");

	dataCenterVMTraversalSpec.name = new string("DataCenterVMTraversalSpec");
	dataCenterVMTraversalSpec.type = "Datacenter";
	dataCenterVMTraversalSpec.path = "vmFolder";
	dataCenterVMTraversalSpec.skip = &xsd_true;

	folderTraversalSpec.name = new string("FolderTraversalSpec");
	folderTraversalSpec.type = "Folder";
	folderTraversalSpec.path = "childEntity";
	folderTraversalSpec.skip = &xsd_true;

	dataCenterVMTraversalSpec.selectSet.push_back(&folderTraversalSelectionSpec);
	folderTraversalSpec.selectSet.push_back(&dataCenterVMTraversalSelectionSpec);
	folderTraversalSpec.selectSet.push_back(&folderTraversalSelectionSpec);

	propertySpec.type = "VirtualMachine";
	propertySpec.all = &xsd_false;
	propertySpec.pathSet.push_back("name");

	objectSpec.obj = serviceContent->rootFolder;
	objectSpec.skip = &xsd_true;
	objectSpec.selectSet.push_back(&folderTraversalSpec);
	objectSpec.selectSet.push_back(&dataCenterVMTraversalSpec);

	propertyFilterSpec.propSet.push_back(&propertySpec);
	propertyFilterSpec.objectSet.push_back(&objectSpec);

	retrievePropertiesReq._USCOREthis = serviceContent->propertyCollector;
	retrievePropertiesReq.specSet.push_back(&propertyFilterSpec);

	if (vim->__ns1__RetrieveProperties(&retrievePropertiesReq, &retrievePropertiesRes) == SOAP_OK) {
		for (int i = 0; i < retrievePropertiesRes.returnval.size(); i++) {
			dynamicProperty = *retrievePropertiesRes.returnval[i]->propSet[0];

			//      assert(dynamicProperty.name == "name");
			xsd__string* name = (xsd__string*) dynamicProperty.val;

			if (vm_name == name->__item) {
				vm_mor = retrievePropertiesRes.returnval[i]->obj;
				break;
			}
		}
	}
	else {
		soap_print_fault(vim->soap, stderr);
	}

	delete folderTraversalSelectionSpec.name;
	delete folderTraversalSpec.name;
	delete dataCenterVMTraversalSelectionSpec.name;
	delete dataCenterVMTraversalSpec.name;

	return vm_mor;
}


static void
powerVM(VimBinding* vim, ns1__ServiceContent* serviceContent,
		string vm_name, bool powerOn) {

	ns1__ManagedObjectReference* vm_mor =
			getVMManagedObjectRef(vim, serviceContent, vm_name);

	if (powerOn) {
		// Power-on the Virtual Machine.
		ns1__PowerOnVMRequestType powerOnReq;
		_ns1__PowerOnVM_USCORETaskResponse powerOnRes; // This line is changed

		powerOnReq._USCOREthis = vm_mor;
		if (vim->__ns1__PowerOnVM_USCORETask(&powerOnReq, &powerOnRes) == SOAP_OK) { // This line is changed
			cout << "PowerOn of VM " << vm_name << " successful" << endl << endl;
		}
		else {
			cout << "PowerOn of VM " << vm_name << " unsuccessful" << endl << endl;
			soap_print_fault(vim->soap,stderr);
			soap_done(vim->soap);
			soap_end(vim->soap);
			exit(1);
		}
	}
	else {
		// Power-off the Virtual Machine.
		ns1__PowerOffVMRequestType powerOffReq;
		_ns1__PowerOffVM_USCORETaskResponse powerOffRes;  // This line is changed

		powerOffReq._USCOREthis = vm_mor;
		if (vim->__ns1__PowerOffVM_USCORETask(&powerOffReq, &powerOffRes) == SOAP_OK) { // This line is changed
			cout << "PowerOff of VM " << vm_name << " successful" << endl << endl;
		}
		else {
			cout << "PowerOff of VM " << vm_name << " unsuccessful" << endl << endl;
			soap_print_fault(vim->soap,stderr);
			soap_done(vim->soap);
			soap_end(vim->soap);
			exit(1);
		}
	}
}


int dumpVMs(VimBinding* vim, ns1__ServiceContent* serviceContent)
{
	bool xsd_true = 1;
	bool xsd_false = 0;

	/* Create the container view for virtual machines */
	ns1__CreateContainerViewRequestType containerRequestType;
	containerRequestType._USCOREthis = serviceContent->viewManager;
	containerRequestType.container = serviceContent->rootFolder;
	containerRequestType.type.push_back("VirtualMachine");
	containerRequestType.recursive = true;

	_ns1__CreateContainerViewResponse containerRequestResponse;

	if (vim->__ns1__CreateContainerView(&containerRequestType, &containerRequestResponse) != SOAP_OK)
	{
		exit(1);
	}

	/* Create the object spec to define the beggining of the traversal at our view */
	ns1__ObjectSpec objectSpec;
	objectSpec.obj = containerRequestResponse.returnval;
	objectSpec.skip = &xsd_true;

	ns1__TraversalSpec traversalSpec;
	traversalSpec.name = new string("traverseEntities");
	traversalSpec.path = "view";
	traversalSpec.skip = &xsd_false;
	traversalSpec.type = "ContainerView";

	objectSpec.selectSet.push_back(&traversalSpec);

	/* specify what property we want */
	ns1__PropertySpec propertySpec;
	propertySpec.type = "VirtualMachine";
	propertySpec.pathSet.push_back("name");
	propertySpec.pathSet.push_back("guest.ipAddress");
	propertySpec.pathSet.push_back("guest.guestState");
	propertySpec.all = &xsd_false;

	// add this to the main request filter spec
	ns1__PropertyFilterSpec propertyFilterSpec;
	propertyFilterSpec.reportMissingObjectsInResults = &xsd_false;
	propertyFilterSpec.propSet.push_back(&propertySpec);
	propertyFilterSpec.objectSet.push_back(&objectSpec);

	// and execute
	ns1__RetrievePropertiesExRequestType request;
	ns1__RetrieveOptions options;

	request._USCOREthis = serviceContent->propertyCollector;
	request.specSet.push_back(&propertyFilterSpec);
	request.options = &options;

	_ns1__RetrievePropertiesExResponse retrievePropertiesRes;

	if (vim->__ns1__RetrievePropertiesEx(&request, &retrievePropertiesRes) == SOAP_OK)
	{
		cout << "Number of Objects " << retrievePropertiesRes.returnval->objects.size() << endl;

		ns1__ObjectContent* oc = NULL;
		ns1__DynamicProperty* dynamicProperty;
		for (int i=0; i < retrievePropertiesRes.returnval->objects.size(); i++)
		{
			oc = retrievePropertiesRes.returnval->objects[i];
			cout << " Object Type: " << oc->obj->type->c_str();
			cout << " (" << oc->propSet.size() << " properties), ";
			for (int j=0; j < oc->propSet.size(); j++)
			{
				dynamicProperty = oc->propSet[j];

				if ( dynamicProperty->name == "name" )
				{
					xsd__string* name = (xsd__string *) dynamicProperty->val;
					cout << " Name: " << name->__item;
				}

				if ( dynamicProperty->name == "guest.ipAddress" )
				{
					xsd__string* name = (xsd__string *) dynamicProperty->val;
					cout << " IP Address: " << name->__item;
				}

				if ( dynamicProperty->name == "guest.guestState" )
				{
					xsd__string* name = (xsd__string *) dynamicProperty->val;
					cout << " Guest State: " << name->__item;
				}

			}
			cout << endl;
		}
	}

	return 0;
}

static void
serverLogout(VimBinding* vim, ns1__ServiceContent* serviceContent) {
	ns1__LogoutRequestType logoutReq;
	_ns1__LogoutResponse logoutRes;

	logoutReq._USCOREthis = serviceContent->sessionManager;

	if (vim->__ns1__Logout(&logoutReq, &logoutRes) == SOAP_OK) {
		cout << "Logout - OK" << endl << endl;
	}
	else
	{
		cout << "Logout - NOT OK" << endl << endl;
		soap_print_fault(vim->soap,stderr);
		soap_done(vim->soap);
		soap_end(vim->soap);
		exit(1);
	}
}
static void
serverLogin(VimBinding* vim, ns1__ServiceContent* serviceContent) {
	ns1__LoginRequestType loginReq;
	_ns1__LoginResponse loginRes;

	loginReq._USCOREthis = serviceContent->sessionManager;
	loginReq.userName = "root";
	loginReq.password = "1234567890";

	if (vim->__ns1__Login(&loginReq, &loginRes) == SOAP_OK) {
		ns1__UserSession* userSession = loginRes.returnval;
		cout << "Login Successful: " << userSession->userName << endl;
	}
	else
	{
		cout << "Login - NOT OK" << endl << endl;
		soap_print_fault(vim->soap,stderr);
		soap_done(vim->soap);
		soap_end(vim->soap);
		exit(1);
	}
}


int main(int argc, char*argv[])
{
	VimBinding vim;
	ns1__ManagedObjectReference ManagedObjectRef;
	ns1__RetrieveServiceContentRequestType RetrieveServiceContentReq;

	_ns1__RetrieveServiceContentResponse RetrieveServiceContentRes;
	ns1__ServiceContent *ServiceContent;
	ns1__AboutInfo *AboutInfo;
	ns1__LoginRequestType LoginReq;
	_ns1__LoginResponse LoginRes;
	ns1__UserSession *UserSession;
	string vm_name;

	if (argc<2){
		cout << "No parameters Provided !" <<endl;
		exit(1);
	}

	string vm_op(argv[1]);
	if (argc==3)
		vm_name=argv[2];

	char* service_url = "https://esxi-server/sdk";

	vim.endpoint = service_url;

	soap_init(vim.soap);
#ifdef DEBUG
	soap_set_recv_logfile(vim.soap, "service12.log"); // append all messages received in /logs/recv/service12.log
	soap_set_sent_logfile(vim.soap, "service13.log"); // append all messages sent in /logs/sent/service13.log
	soap_set_test_logfile(vim.soap, "test.log"); // append all DEBUG messages in /logs/sent/test.log
#endif

	soap_ssl_init();
	if (soap_ssl_client_context( vim.soap, SOAP_SSL_NO_AUTHENTICATION,
								NULL, NULL, NULL, NULL, NULL ))
	{
		cout << "SSL:";
		soap_print_fault(vim.soap, stderr);
		exit(1);
	}

	ManagedObjectRef.__item = "ServiceInstance";
	ManagedObjectRef.type = new string("ServiceInstance");
	RetrieveServiceContentReq._USCOREthis = &ManagedObjectRef;

	if ( vim.__ns1__RetrieveServiceContent(&RetrieveServiceContentReq, &RetrieveServiceContentRes) == SOAP_OK )
	{
		cout << "RetrieveServiceContent - OK" << endl;
	}
	else
	{
		soap_print_fault(vim.soap,stderr);
		exit(1);
	}

	ServiceContent = RetrieveServiceContentRes.returnval;

	if (ServiceContent && ServiceContent->about)
	{
		AboutInfo = ServiceContent->about;
		cout << "fullName: " << AboutInfo->fullName << endl;
	}

	if (ServiceContent && ServiceContent->sessionManager)
	{
		serverLogin(&vim, ServiceContent);

		if (vm_op=="list")
			if ( dumpVMs(&vim, ServiceContent) == 0)
			{
				cout << "dumpVMs Successful: " << endl;
				exit(0);
			}
			else
			{
				soap_print_fault(vim.soap, stderr);
				exit(1);
			}

		if (argc==3)
		{
			if (vm_op == "powerOn") {
				// Power-on the specified VM.
				powerVM(&vim, ServiceContent, vm_name, true);
			}
			else if (vm_op == "powerOff") {
				// Power-off the specified VM.
				powerVM(&vim, ServiceContent, vm_name, false);
			}
		}
		delete ManagedObjectRef.type;
	}
	serverLogout(&vim, ServiceContent);

	soap_done(vim.soap);
	soap_end(vim.soap);

	return 0;
}
