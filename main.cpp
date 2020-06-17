#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
using std::wstring;
using std::wcin;

int main( int args, char* argv[]) {

    if( args != 2 ) {
        printf( "Provide online 1 argument [ file path ]\n" );
        system("pause");
        return 0;
    }
    
    //path
    //const char* filePath = "example.exe";
    
    //console path
    const char* filePath = argv[1]; 

    //load the file into memory
    std::ifstream streamBuffer( filePath, std::ios::binary | std::ios::ate );
    size_t fileSize = static_cast<size_t>( streamBuffer.tellg() );
    printf("Size of file %i\n", fileSize );
    BYTE* pLocalBuffer = new BYTE[fileSize];

    if( !pLocalBuffer ) {
        printf( "Error allocationg local buffer\n" );
        system("pause");
        return 0;
    }

    streamBuffer.seekg( 0, std::ios::beg );
    streamBuffer.read( reinterpret_cast<char*>( pLocalBuffer ), fileSize );
    streamBuffer.close();


    //setup header pointers
    auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>( pLocalBuffer );
    auto pNtHeader  = reinterpret_cast<PIMAGE_NT_HEADERS>( pLocalBuffer + pDosHeader->e_lfanew );

    //check if there are imports
    if( !( pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size )) {
        printf( "No imports found\n" );
        system("pause");
        return 0;
    }

    PIMAGE_IMPORT_DESCRIPTOR pImportDirectory = nullptr;
    DWORD importRVA = 0;
    DWORD importBaseOffset = 0;
    
    importRVA = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    
    PIMAGE_SECTION_HEADER sectionArray = reinterpret_cast<PIMAGE_SECTION_HEADER>( pNtHeader + 1 );

    int numOfSections = pNtHeader->FileHeader.NumberOfSections;

    PIMAGE_SECTION_HEADER pNameSectionHeader = nullptr; 

    //Import Directory is contained in one of the section usually .idata section
    //we cant relay on hardcoded data
    for( int i = 0; i != numOfSections; ++i ) {
        
        //to check if current section contains our Import Directory, make sure Import Directory RVA is between current section RVA and 
        //size of raw data. 

        if( importRVA >= sectionArray[i].VirtualAddress && ( importRVA < sectionArray[i].VirtualAddress + sectionArray[i].SizeOfRawData )) {
            printf( "Found import dir in [%s] section\n", sectionArray[i].Name );
            pNameSectionHeader = &sectionArray[i];

            importBaseOffset = ( importRVA - sectionArray[i].VirtualAddress + sectionArray[i].PointerToRawData );
            pImportDirectory = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>( pLocalBuffer + importBaseOffset );
            break;
        }
    }


    //since the array is nullterminated we can do this
    while( pImportDirectory->Name ) {
        DWORD moduleNameRVA = pImportDirectory->Name;

        DWORD moduleNameOffset = 0;
        moduleNameOffset = ( moduleNameRVA - pNameSectionHeader->VirtualAddress + pNameSectionHeader->PointerToRawData );
        printf( "\n\t\t%s\n\n", reinterpret_cast<char*>( pLocalBuffer + moduleNameOffset ) );

        DWORD nameImportOffset = pImportDirectory->OriginalFirstThunk;
        DWORD nameAddressOffset = pImportDirectory->FirstThunk; 

        //find thunk arrays raw data offsets , essentialy this are INT( import name table ), and IAT ( import address table )
        nameImportOffset = nameImportOffset - pNameSectionHeader->VirtualAddress + pNameSectionHeader->PointerToRawData;
        nameAddressOffset = nameAddressOffset - pNameSectionHeader->VirtualAddress + pNameSectionHeader->PointerToRawData;
//
        //add raw data offset to base to get actual addresses of INT and IAT
        ULONG_PTR* pNameImport    = reinterpret_cast<ULONG_PTR*>( pLocalBuffer + nameImportOffset );
        ULONG_PTR* pAddressImport = reinterpret_cast<ULONG_PTR*>( pLocalBuffer + nameAddressOffset );

        if(  !(*pNameImport)  ) {
            pNameImport = pAddressImport;
        }

        for(; *pNameImport; pNameImport++, pAddressImport++ ) {
            if( IMAGE_SNAP_BY_ORDINAL( *pNameImport ) ) {
                printf( "[%x] Unknown function\n", *pNameImport & 0xFFFF );
            } else {
                PIMAGE_IMPORT_BY_NAME pImportFunction = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>( pLocalBuffer + ( *pNameImport - pNameSectionHeader->VirtualAddress + pNameSectionHeader->PointerToRawData ) );
                printf( "\t[%x] [%s]\n", pImportFunction->Hint, reinterpret_cast<char*>( pImportFunction->Name) );
            }
        }
        pImportDirectory++;
    }

    delete[] pLocalBuffer;
    system("pause");
    return 0;
}
