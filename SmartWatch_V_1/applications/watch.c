/*********************************************** *******************************
* File Name          : watch.c
* Author             : ElHombreSoqo
* Version            : V1.0.0
* Date               : 07 Sep 2022
* Description        : Main threads and functionality
*******************************************************************************/
#include "watch.h"
#include <math.h>       //For sqrt
#include <stdlib.h>     //For itoa

/* Global Variables */

uint16_t Theme;         //To change front color
uint16_t Background;    //To change background color

/////////* RTC *///////////
time_t now;             //To get initial values of RTC
char *string;           //To manipulate RTC Numbers
///////////////////////////

///////////* Step Counter (MPU-6050) *////////
int steps = 0;          //Step counter
char stepString[20];    //Variable to change an Int to String (To show on TFT)

const int threshold = 10500;    //Arbitrary number to measure steps (This is the best number i got from my tests)

short aacx,aacy,aacz;   //Accelerometer values
//////////////////////////////////////////////

//////////* ADC Batt Meas *////////////////
uint16_t Batt;          //Batt value
char battString[20];    //Variable to change an Int to String (To show on TFT)
///////////////////////////////////////////

/////////* Physical button counters for menus *////////////////
int secondsbutton=0, minutesbutton=0, hoursbutton=0, monthbutton=0, daysbutton=0, menu_index = 0, themeChanged = 0;
///////////////////////////////////////////////////////////////

/// /////////* Time variables *////////////////
int Hour=0, Minute=0, Second=0, Month=1, Day=1;
char Hour_str[2], Minute_str[2], Second_str[2], Day_str[2];
///////////////////////////////////////////

/***************** Thread initializations ***************/
static rt_thread_t getValues = RT_NULL;     //Get values from sensors  (MEDIUM)
static rt_thread_t showValues = RT_NULL;    //Display values on screen (HIGH)
//////////////////////////////////////////////////////////

/***************** Used sync methods *******************/
static rt_mutex_t dynamic_mutex  = RT_NULL;
static rt_sem_t dynamic_sem = RT_NULL;
/////////////////////////////////////////////////////////

/***************** Tread entry functions **************/
static void serial_thread_entry(void *parameter);
static void getValues_entry(void *parameter);
static void showValues_entry(void *parameter);
/////////////////////////////////////////////////////////


static rt_device_t serial;                  //Serial device Init (RT-Thread Device)

int rtc_sample();
u16 Get_ADC_Val(u8 ch);
void up_btn (void *args);
void down_btn (void *args);
void select_btn (void *args);
static rt_err_t uart_input(rt_device_t dev, rt_size_t size);


int Main_Threads()
{
    char uart_name[RT_NAME_MAX];
    char str[] = "hello RT-Thread!\r\n";

    rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);

    /* Find uart devices in the system */
    serial = rt_device_find(uart_name);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* Initial theme */
    Theme = LCD_DISP_GREEN;
    Background = LCD_DISP_BLACK;

    /* Create a dynamic mutex */
    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
        if (dynamic_mutex == RT_NULL)
        {
            rt_kprintf("create dynamic mutex failed.\n");
            return -1;
        }

    /* Create a dynamic semaphore */
    dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);
        if (dynamic_sem == RT_NULL)
        {
            rt_kprintf("create dynamic semaphore failed.\n");
            return -1;
        }
        else
        {
            rt_kprintf("create done. dynamic semaphore value = 0.\n");
        }


       /* Open the uart device in interrupt receive and polling send mode */
       rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
       /* Set the receive callback function */
       rt_device_set_rx_indicate(serial, uart_input);
       /* Send string */
       rt_device_write(serial, 0, str, (sizeof(str) - 1));

       /* Create a serial thread */
       rt_thread_t uart = rt_thread_create("serial",
                                             serial_thread_entry,
                                             RT_NULL,
                                             THREAD_STACK_SIZE,
                                             THREAD_PRIORITY_LOW,
                                             THREAD_TIMESLICE_UART);

       /* Start the thread successfully */
       if (uart != RT_NULL)
           rt_thread_startup(uart);

       /* Create getValues thread */
        getValues = rt_thread_create("Get",
                                    getValues_entry, (void*)1,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY_MED, THREAD_TIMESLICE);

        /* Start the thread successfully */
        if (getValues != RT_NULL)
        rt_thread_startup(getValues);

        /* Create showValues thread */
        showValues = rt_thread_create("Show",
                                    showValues_entry, (void*)2,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY_HIGH, THREAD_TIMESLICE);

        /* Start the thread successfully */
        if (showValues != RT_NULL)
        rt_thread_startup(showValues);

    return 0;
}

/* Entry for UART Comms */
static void serial_thread_entry(void *parameter)
{
    char ch;
    char str2[] = "Color Changed\r\n";

    while (1)
    {
        /* Read a byte of data from the serial port and wait for the receiving semaphore if it is not read */
        while (rt_device_read(serial, -1, &ch, 1) != 1)
        {
            /* Being Suspended and waiting for the semaphore */
            rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        }

        /* Read the data from the serial port and output through dislocation */
        rt_device_write(serial, 0, &ch, 1);
        switch(ch)
        {
            case 'R':                       //RED
                Theme = LCD_DISP_RED;
                themeChanged = 1;
            break;

            case 'G':                       //GREEN
                Theme = LCD_DISP_GREEN;
                themeChanged = 1;
            break;

            case 'B':                       //BLUE
                Theme = LCD_DISP_BLUE;
                themeChanged = 1;
            break;

            case 'Y':                       //YELLOW
                Theme = LCD_DISP_YELLOW;
                themeChanged = 1;
            break;

            case 'C':                       //CYAN
                Theme = LCD_DISP_CYAN;
                themeChanged = 1;
            break;

            case 'M':                       //MAGENTA
                Theme = LCD_DISP_MAGENTA;
                themeChanged = 1;
            break;

            case 'O':                       //ORANGE
                Theme = LCD_DISP_ORANGE;
                themeChanged = 1;
            break;

            case 'W':                       //WHITE
                Theme = LCD_DISP_WHITE;
                themeChanged = 1;
            break;

            default:
                rt_device_write(serial, 0, str2, (sizeof(str2) - 1));
        }

    }
}

/* Entry Function for getValues */
static void getValues_entry(void *parameter)
{
      /* UP_PIN_IRQ_Init  */
      rt_pin_mode(UP_PIN, PIN_MODE_INPUT_PULLUP);
      rt_pin_attach_irq(UP_PIN, PIN_IRQ_MODE_FALLING, up_btn, RT_NULL);
      rt_pin_irq_enable(UP_PIN, PIN_IRQ_ENABLE);

      /* SELECT_PIN_IRQ_Init  */
      rt_pin_mode(SELECT_PIN, PIN_MODE_INPUT_PULLUP);
      rt_pin_attach_irq(SELECT_PIN, PIN_IRQ_MODE_FALLING, select_btn, RT_NULL);
      rt_pin_irq_enable(SELECT_PIN, PIN_IRQ_ENABLE);

     /* DOWN_PIN_IRQ_Init */
      rt_pin_mode(DOWN_PIN, PIN_MODE_INPUT_PULLUP);
      rt_pin_attach_irq(DOWN_PIN, PIN_IRQ_MODE_FALLING, down_btn, RT_NULL);
      rt_pin_irq_enable(DOWN_PIN, PIN_IRQ_ENABLE);

    while (1)
    {
        //////* Get time */////////
        now = time(RT_NULL);
        string = ctime(&now);
        //////////////////////////

        ////* Get accelerometer values and transform them to a string *///
        uint16_t vectorprevious = 0;
        uint16_t vector = 0;
        uint16_t totalvector = 0;

        MPU_Get_Accelerometer(&aacx,&aacy,&aacz);
        rt_thread_mdelay(3);

        vector = sqrt( (aacx * aacx) + (aacy * aacy) + (aacz * aacz) );
        totalvector = (vector - vectorprevious)/2;

        if (totalvector > threshold){
           steps++;
           rt_thread_mdelay(500);           //Step Timeout
        }

        itoa(steps,stepString,10);          //Turn step value (int) to string

        /////////////////////////////////////////////////////////////////

        ///////////* Batt values (LiPo) */////////////////

        Batt=Get_ADC_Val(1);                //ADC MAX Voltage = 2.1V    | MAX ADC value = 2606 (4.2V -> 2.1V Voltage divider)
        Batt=Batt-1800;                     //ADC MIN Voltage = 1.45V   | MIN ADC value = 1800 (2.9V -> 1.45V Voltage divider)
        Batt= (Batt*100)/806;               //Range goes from 100% (2.1V) to 0% (1.45V)

        //Batt= (Batt*100)/2606;

        itoa(Batt-1,battString,10);         //Turn Batt value (int) to string

        //////////////////////////////////////////////////

        rt_thread_mdelay(2);
        rt_sem_release(dynamic_sem);
    }
}


/* Entry Function for showValues */
static void showValues_entry(void *parameter)
{
    static rt_err_t semaphore;

    /* These simbols only print once (no need to print them periodically)*/
    WriteString(210, 10, "%", Font_7x10, Theme, Background);
    rt_thread_mdelay(1);
    WriteString(85, 180, "!", Clock_font, Theme, Background);   //Modified font to show steps symbol
    rt_thread_mdelay(1);

    while (1)
    {
        semaphore = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);

            if (semaphore != RT_EOK)
            {

                rt_kprintf("t2 take a dynamic semaphore, failed.\n");
                rt_sem_delete(dynamic_sem);
                return;

            }else{

                if(themeChanged == 1)
                {

                    WriteString(210, 10, "%", Font_7x10, Theme, Background);
                    rt_thread_mdelay(1);
                    WriteString(85, 180, "!", Clock_font, Theme, Background);   //Modified font to show steps symbol
                    rt_thread_mdelay(1);
                    themeChanged = 0;

                }

                switch (menu_index) //Each time the "SELECT" button is pressed, a different selection can be edited (edit minutes, edit seconds, etc.)
                {
                 case 0:
                     /* Show Basic screen */
                     WriteString(190, 10, battString, Font_7x10, Theme, Background);     //Batt value
                     rt_thread_mdelay(1);
                     RTC_ST7789_Time(40, 60, string, Clock_font, Theme, Background);     //Hour - Minute - Second
                     rt_thread_mdelay(1);
                     RTC_ST7789_Date(65, 120, string, Font_11x18, Theme, Background);    //Day of the week - Month - Day
                     rt_thread_mdelay(1);
                     WriteString(105, 180, stepString, Font_11x18, Theme, Background);   //Steps
                     rt_thread_mdelay(1);
                 break;

                 case 1:
                      itoa(Hour,Hour_str,10);                                             //Turn Hour value (int) to string

                    if(hoursbutton < 10)
                    {
                        WriteString(40, 60, "0", Clock_font, Background, Theme);         //If the hour value is below 10, this puts a 0 before the selected value
                        rt_thread_mdelay(1);
                        WriteString(60, 60, Hour_str, Clock_font, Background, Theme);    //Hour value  Format: 00, 01, 02, etc.
                        rt_thread_mdelay(1);

                    }else{
                        WriteString(40, 60, Hour_str, Clock_font, Background, Theme);      //Hour value
                        rt_thread_mdelay(1);
                    }
                 break;

                 case 2:
                      itoa(Minute,Minute_str,10);                                         //Turn Minute value (int) to string

                      //////// Clear the previous selection //////////////
                      if(hoursbutton < 10)
                      {
                          WriteString(40, 60, "0", Clock_font, Theme, Background);
                          rt_thread_mdelay(1);
                          WriteString(60, 60, Hour_str, Clock_font, Theme, Background);
                          rt_thread_mdelay(1);
                      }else{
                          WriteString(40, 60, Hour_str, Clock_font, Theme, Background);
                          rt_thread_mdelay(1);
                      }
                      ////////////////////////////////////////////////////

                     if(minutesbutton < 10)
                     {
                         WriteString(100, 60, "0", Clock_font, Background, Theme);        //If the minute value is below 10, this puts a 0 before the selected value
                         rt_thread_mdelay(1);
                         WriteString(120, 60, Minute_str, Clock_font, Background, Theme); //Minutes value  Format: 00, 01, 02, etc.
                         rt_thread_mdelay(1);

                     }else{
                         WriteString(100, 60, Minute_str, Clock_font, Background, Theme); //Minutes value
                         rt_thread_mdelay(1);
                     }
                  break;

                 case 3:
                       itoa(Second,Second_str,10);                                         //Turn Minute value (int) to string

                       //////// Clear the previous selection //////////////
                       if(minutesbutton < 10)
                       {
                           WriteString(100, 60, "0", Clock_font, Theme, Background);
                           rt_thread_mdelay(1);
                           WriteString(120, 60, Minute_str, Clock_font, Theme, Background);
                           rt_thread_mdelay(1);
                       }else{
                           WriteString(100, 60, Minute_str, Clock_font, Theme, Background);
                           rt_thread_mdelay(1);
                       }
                       ////////////////////////////////////////////////////

                      if(secondsbutton < 10)
                      {
                          WriteString(160, 60, "0", Clock_font, Background, Theme);        //If the second value is below 10, this puts a 0 before the selected value
                          rt_thread_mdelay(1);
                          WriteString(180, 60, Second_str, Clock_font, Background, Theme); //Seconds value  Format: 00, 01, 02, etc.
                          rt_thread_mdelay(1);

                      }else{
                          WriteString(160, 60, Second_str, Clock_font, Background, Theme); //Seconds value
                          rt_thread_mdelay(1);
                      }
                 break;

                 case 4:
                     //////// Clear the previous selection //////////////
                     if(secondsbutton < 10)
                     {
                         WriteString(160, 60, "0", Clock_font, Theme, Background);
                         rt_thread_mdelay(1);
                         WriteString(180, 60, Second_str, Clock_font, Theme, Background);
                         rt_thread_mdelay(1);
                     }else{
                         WriteString(160, 60, Second_str, Clock_font, Theme, Background);
                         rt_thread_mdelay(1);
                     }
                     ////////////////////////////////////////////////////

                     switch(monthbutton)
                     {
                     case 1:
                         RTC_ST7789_Date(109, 120, "Jan", Font_11x18, Background, Theme);    //Print Jan on screen
                         rt_thread_mdelay(1);
                     break;
                     case 2:
                         RTC_ST7789_Date(109, 120, "Feb", Font_11x18, Background, Theme);    //Print Feb on screen
                         rt_thread_mdelay(1);
                     break;
                     case 3:
                         RTC_ST7789_Date(109, 120, "Mar", Font_11x18, Background, Theme);    //Print Mar on screen
                         rt_thread_mdelay(1);
                     break;
                     case 4:
                         RTC_ST7789_Date(109, 120, "Apr", Font_11x18, Background, Theme);    //Print Apr on screen
                         rt_thread_mdelay(1);
                     break;
                     case 5:
                         RTC_ST7789_Date(109, 120, "May", Font_11x18, Background, Theme);    //Print May on screen
                         rt_thread_mdelay(1);
                     break;
                     case 6:
                         RTC_ST7789_Date(109, 120, "Jun", Font_11x18, Background, Theme);    //Print Jun on screen
                         rt_thread_mdelay(1);
                     break;
                     case 7:
                         RTC_ST7789_Date(109, 120, "Jul", Font_11x18, Background, Theme);    //Print Jul on screen
                         rt_thread_mdelay(1);
                     break;
                     case 8:
                         RTC_ST7789_Date(109, 120, "Aug", Font_11x18, Background, Theme);    //Print Aug on screen
                         rt_thread_mdelay(1);
                     break;
                     case 9:
                         RTC_ST7789_Date(109, 120, "Sep", Font_11x18, Background, Theme);    //Print Sep on screen
                         rt_thread_mdelay(1);
                     break;
                     case 10:
                         RTC_ST7789_Date(109, 120, "Oct", Font_11x18, Background, Theme);    //Print Oct on screen
                         rt_thread_mdelay(1);
                     break;
                     case 11:
                         RTC_ST7789_Date(109, 120, "Nov", Font_11x18, Background, Theme);    //Print Nov on screen
                         rt_thread_mdelay(1);
                     break;
                     case 12:
                         RTC_ST7789_Date(109, 120, "Dec", Font_11x18, Background, Theme);    //Print Dec on screen
                         rt_thread_mdelay(1);
                     break;

                     default:
                         RTC_ST7789_Date(109, 120, "Jan", Font_11x18, Background, Theme);    //Day of the week - Month - Day
                         rt_thread_mdelay(1);
                     }
                 break;

                 case 5:
                     itoa(Day,Day_str,10);

                     if(daysbutton < 10)
                     {
                         WriteString(153, 120, "0", Font_11x18, Background, Theme);         //If the day value is below 10, this puts a 0 before the selected value
                         rt_thread_mdelay(1);
                         WriteString(164, 120, Day_str, Font_11x18, Background, Theme);     //Days value  Format: 00, 01, 02, etc.
                         rt_thread_mdelay(1);

                     }else{
                         WriteString(153, 120, Day_str, Font_11x18, Background, Theme);     //Days value
                         rt_thread_mdelay(1);
                     }
                 break;

                 default:
                     /* Show Basic screen */
                     WriteString(190, 10, battString, Font_7x10, Theme, Background);
                     rt_thread_mdelay(1);
                     RTC_ST7789_Time(40, 60, string, Clock_font, Theme, Background);
                     rt_thread_mdelay(1);
                     RTC_ST7789_Date(65, 120, string, Font_11x18, Theme, Background);
                     rt_thread_mdelay(1);
                     WriteString(105, 180, stepString, Font_11x18, Theme, Background);
                     rt_thread_mdelay(1);
                }
            }
    }
}

/* Receive data callback function */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* After the uart device receives the data, it generates an interrupt, calls this callback function, and then sends the received semaphore. */
    rt_mutex_release(dynamic_mutex);

    return RT_EOK;
}

/* RTC Sample code from RT-Thread document center. Go to: Device -> RTC Device */
int rtc_sample()
{
    rt_err_t ret = RT_EOK;
    time_t rtc_time;

    /* Set date */
    ret = set_date(2022, Month, Day);
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC date failed\n");
        return ret;
    }

    /* Set time */
    ret = set_time(Hour, Minute, Second);
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC time failed\n");
        return ret;
    }

    /* Delay 500 seconds */
    rt_thread_mdelay(500);

    /* Obtain Time */
    rtc_time = time(RT_NULL);
    rt_kprintf("%s\n", ctime(&rtc_time));

    return ret;
}

/* Get ADC Val fuction from CH32V307 MounRiver examples */
u16 Get_ADC_Val(u8 ch)
{
    u16 val;

    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));

    val = ADC_GetConversionValue(ADC1);

    return val;
}

/* UP button IRQ */
void up_btn (void *args)
{

    switch(menu_index)          //Check which menu to use
    {

    case 1:                     //Add +1 Hour every press
        hoursbutton++;
        if(hoursbutton > 23)
        {
            hoursbutton = 0;
        }
        Hour = hoursbutton;
    break;

    case 2:                     //Add +1 Minute every press
        minutesbutton++;
        if(minutesbutton > 59)
        {
            minutesbutton = 0;
        }
        Minute = minutesbutton;
    break;

    case 3:                     //Add +1 Second every press
        secondsbutton++;
        if(secondsbutton > 59)
        {
            secondsbutton = 0;
        }
        Second = secondsbutton;
    break;

    case 4:                     //Add +1 Month every press
        monthbutton++;
        if(monthbutton > 12)
        {
            monthbutton = 1;
        }
        Month = monthbutton;
    break;

    case 5:
        daysbutton++;           //Add +1 Day every press
        if(daysbutton > 31)
        {
            daysbutton = 1;
        }
        Day = daysbutton;
    break;

    default:
        Hour = hoursbutton;
    }

}

/* SELECT button IRQ */
void select_btn (void *args)
{
    rt_err_t ret = RT_EOK;

    menu_index++;
    if(menu_index > 5)          //If every menu is done
    {

        /* Set time */
        ret = set_time(Hour, Minute, Second);
        /* Set date */
        ret = set_date(2022, Month, Day);
        menu_index = 0;

    }
}

/* DOWN button IRQ */
void down_btn (void *args)
{

    switch(menu_index)
    {

        case 1:                 //-1 Hour every press
            hoursbutton--;
            if(hoursbutton < 0)
            {
                hoursbutton = 0;
            }
            Hour = hoursbutton;
        break;

        case 2:                 //-1 Minute every press
            minutesbutton--;
            if(minutesbutton < 0)
            {
                minutesbutton = 0;
            }
            Minute = minutesbutton;
        break;

        case 3:                 //-1 Second every press
            secondsbutton--;
            if(secondsbutton < 0)
            {
                secondsbutton = 0;
            }
            Second = secondsbutton;
        break;

        case 4:                 //-1 Month every press
            monthbutton--;
            if(monthbutton < 0)
            {
                monthbutton = 1;
            }
            Month = monthbutton;
        break;

        case 5:                 //-1 Day every press
                daysbutton--;
                if(daysbutton < 0)
                {
                    daysbutton = 1;
                }
                Day = daysbutton;
            break;

        default:
            Hour = hoursbutton;
    }

}
