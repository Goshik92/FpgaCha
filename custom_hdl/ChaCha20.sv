/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
* Description:
*     The module implements a ChaCha20 algorithm.
*     It can be connected to Nios II or ARM processor
*     Using Avalon-MM interface
***********************************************/

// Data structures used in the module
typedef logic [31:0] Word_t;
typedef Word_t QState_t[4];
typedef Word_t State_t[16];
typedef logic [3:0] StateIdx_t;

// Rotates left a word
function automatic Word_t RotLeft(Word_t in, logic [4:0] n);
    RotLeft = in << n | in >> (~n + 1'b1); 
endfunction

// Performs one quarter round of ChaCha20
function automatic QState_t QRound(input QState_t in);
    Word_t a = in[0] + in[1];
    Word_t d = RotLeft((in[3] ^ a), 5'd16);
    Word_t c = in[2] + d;
    Word_t b = RotLeft((in[1] ^ c), 5'd12);
    QRound[0] = a + b;
    QRound[3] = RotLeft((d ^ QRound[0]), 5'd8);
    QRound[2] = c + QRound[3];
    QRound[1] = RotLeft((b ^ QRound[2]), 5'd7);
endfunction

// Performs one quarter round on State based on supplied offsets
function automatic State_t QRoundIdx(input State_t s, input StateIdx_t idx[4]);
    QState_t out = QRound('{s[idx[0]], s[idx[1]], s[idx[2]], s[idx[3]]});
    QRoundIdx = s;
    for(int i = 0; i < 4; i++) QRoundIdx[idx[i]] = out[i];
endfunction

// Does the summation step
function automatic State_t SumStates(State_t s1, State_t s2);
    for(int i = 0; i < $size(State_t); i++) begin
        SumStates[i] = s1[i] + s2[i];
    end
endfunction

// Does one even round
function automatic State_t EvenRound(input State_t s);
    s = QRoundIdx(s, '{0, 4,  8, 12});
    s = QRoundIdx(s, '{1, 5,  9, 13});
    s = QRoundIdx(s, '{2, 6, 10, 14});
    s = QRoundIdx(s, '{3, 7, 11, 15});
    EvenRound = s;
endfunction

// Does one odd round
function automatic State_t OddRound(input State_t s);
    s = QRoundIdx(s, '{0, 5, 10, 15});
    s = QRoundIdx(s, '{1, 6, 11, 12});
    s = QRoundIdx(s, '{2, 7,  8, 13});
    s = QRoundIdx(s, '{3, 4,  9, 14});
    OddRound = s;
endfunction

module ChaCha20(
    input logic clock,
    input logic reset,
    
    // Avalon-MM slave interface for accessing control-status registers
    input logic csr_read,
    input logic csr_write,
    input logic [5:0] csr_address,
    output logic [31:0] csr_readdata,
    input logic [31:0] csr_writedata
);

    // Round being computed currently
    logic [3:0] roundCounter;
    
    // ChaCha20 states
    // roundResult is not actually a register and is used in a MUX
    State_t initState, finalState, roundResult;

    always_ff @(posedge clock) begin
        if (reset) begin
            roundCounter <= 1'b0;
        end
        
        else begin
            // Choose source for a double round (init state or current state)
            roundResult = OddRound(EvenRound(csr_write ? initState : finalState));
        
            // Avalon write operation
            if (csr_write) casez(csr_address)
                
                // Fill initial state
                6'b00????: initState[csr_address[3:0]] <= csr_writedata;
                
                // If CONTROL register is written, start computation
                6'b100000: begin
                    roundCounter <= csr_writedata[3:0];
                    finalState <= roundResult;
                end
            endcase
            
            // If one of the rounds is beign computed
            else if (roundCounter > 1'b1) begin
                finalState <= roundResult;
                roundCounter <= roundCounter - 1'b1;
            end
            
            // If it is the summation stage now
            else if (roundCounter == 1'b1) begin
                finalState <= SumStates(initState, finalState);
                roundCounter <= roundCounter - 1'b1;
            end
            
            // Reading registers
            if (csr_read) casez(csr_address)
                6'b00????: csr_readdata <= initState[csr_address[3:0]];
                6'b01????: csr_readdata <= finalState[csr_address[3:0]];
                6'b100000: csr_readdata <= roundCounter;
            endcase
        end
    end
    
    
endmodule