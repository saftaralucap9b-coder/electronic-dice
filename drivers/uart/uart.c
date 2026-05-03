#include <avr/io.h>

void UART_Init(uint32_t baud) {
    // Calculăm viteza (pentru 16MHz)
    uint16_t ubrr = (16000000UL / 16 / baud) - 1;
    
    // Setăm registrele de viteză (Baud Rate)
    // Folosim adresele directe dacă numele nu sunt recunoscute
    *((volatile uint8_t *)(0xC5)) = (uint8_t)(ubrr >> 8); // UBRR0H
    *((volatile uint8_t *)(0xC4)) = (uint8_t)ubrr;        // UBRR0L
    
    // Activăm Transmisia (TXEN0 este bitul 3 în UCSR0B)
    *((volatile uint8_t *)(0xC1)) = (1 << 3); 
    
    // Format: 8 biți date, 1 bit stop (UCSZ01, UCSZ00 sunt biții 2 și 1 în UCSR0C)
    *((volatile uint8_t *)(0xC2)) = (1 << 2) | (1 << 1);
}

void UART_SendChar(char c) {
    // Așteptăm buffer gol (UDRE0 este bitul 5 în UCSR0A)
    while (!(*((volatile uint8_t *)(0xC0)) & (1 << 5)));
    
    // Trimitem caracterul (UDR0)
    *((volatile uint8_t *)(0xC6)) = c;
}