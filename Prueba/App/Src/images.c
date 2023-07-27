#include "images.h"
#include "stdint.h"
void imageSelector(ILI9341_Handler_t *ili9341_handler,uint16_t num){
switch (num) {
    case 1:
        Ili_starCom(ili9341_handler);
        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
        printImage1C(ili9341_handler);
        Ili_endCom(ili9341_handler);
        break;
    case 2:
        Ili_starCom(ili9341_handler);
        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
        printImage2C(ili9341_handler);
        Ili_endCom(ili9341_handler);
        break;
    case 3:
        Ili_starCom(ili9341_handler);
        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
        printImage3C(ili9341_handler);
        Ili_endCom(ili9341_handler);
        break;
//    case 4:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage4C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 5:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage5C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 6:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage6C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 7:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage7C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 8:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage8C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 9:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage9C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 10:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage10C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 11:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage11C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 12:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage12C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 13:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage13C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 14:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage14C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
//    case 15:
//        Ili_starCom(ili9341_handler);
//        ILI9341_SetAddressWindow(ili9341_handler, 0, 0, 320 - 1, 240 - 1);
//        printImage15C(ili9341_handler);
//        Ili_endCom(ili9341_handler);
//        break;
    default:
        break;
}
}
